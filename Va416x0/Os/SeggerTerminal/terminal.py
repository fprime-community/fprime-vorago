#!/usr/bin/python3
# Copyright 2025 California Institute of Technology
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

import argparse
import asyncio
import datetime
import os
import sys
import time
from typing import Union, Optional
from jlinksdk import JLinkError

import aiofile

import Va416x0.Os.SeggerTerminal.config as config

try:
    import jlinksdk
except ImportError:
    print("\033[31m", file=sys.stderr)
    print(
        "This library depends on the J-Link SDK being installed, but it could not be found."
        "If you are seeing this error, it means that your build environment does not have the proper dependencies installed."
        "Please ensure that the J-Link SDK is included in your Dockerfile or similar build environment setup script."
        "Ask if your organization has a license to the SDK.",
        file=sys.stderr,
    )
    print("\033[0m", file=sys.stderr)
    raise

# We can hard-code this address (and avoid needing to scan through a binary) because va416x0.ld enforces that the
# SEGGER RTT block is always located at the start of RAM.
SEGGER_RTT_BLOCK_ADDRESS = 0x1FFF8000

# Default format for (optional) beginning of line timestamp
DEFAULT_TIME_FMT = "[%Y%m%dT%H%M%S]"


class OutputStreamWriter:
    def __init__(
        self,
        stdout: asyncio.StreamWriter,
        output_file: Optional[str],
        time_fmt: Optional[str],
    ):
        self.output_file = output_file
        self._file = stdout
        self.time_fmt = time_fmt
        self.write_buffer = []

    async def __aenter__(self):
        if self.output_file:
            try:
                self._file = await aiofile.async_open(self.output_file, "wb")
                print(f"Directing output to {os.path.abspath(self.output_file)}")
            except OSError as e:
                sys.exit(f"\nERROR: Unable to open {self.output_file!r} - {e!r}")
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        if not isinstance(self._file, asyncio.StreamWriter):
            await self._file.close()

    async def awrite(self, data: bytes):
        self.write(data)
        await self.drain()

    def write(self, data: bytes):
        self.write_buffer.append(data)

    async def drain(self):
        for data in self.write_buffer:
            if self.time_fmt:
                data = data.replace(b"\n", get_timestamp(self.time_fmt))
            if isinstance(self._file, asyncio.StreamWriter):
                self._file.write(data)
                await self._file.drain()
            else:
                await self._file.write(data)
                await self._file.flush()

        del self.write_buffer[:]


def get_timestamp(time_fmt: str) -> bytes:
    return datetime.datetime.now().strftime(f"\n{time_fmt}  ").encode()


async def copy_stream_to_stream(
    input: asyncio.StreamReader,
    output: Union[asyncio.StreamWriter, OutputStreamWriter],
):
    # When there's a long running JLink command executed by another
    # program (such as loadbin), the stream read can hang instead of
    # accepting new input
    # So use a timeout and keep running the loop until a successful
    # read returns no data
    # NOTE: I considered using jlink.IsOpen() and/or jlink.IsConnected()
    # NOTE: to check if the connection was still valid, but that added
    # NOTE: enough delay data was dropped, so not doing that
    while True:
        try:
            # Data gets dropped when the wait duration is too small
            data = await asyncio.wait_for(input.read(4096), 5)
            if not data:
                break
        except asyncio.TimeoutError:
            continue
        else:
            output.write(data)
            await output.drain()


# From https://stackoverflow.com/a/64317899
async def connect_stdin_stdout():
    loop = asyncio.get_event_loop()
    reader = asyncio.StreamReader()
    protocol = asyncio.StreamReaderProtocol(reader)
    await loop.connect_read_pipe(lambda: protocol, sys.stdin)
    w_transport, w_protocol = await loop.connect_write_pipe(
        asyncio.streams.FlowControlMixin, sys.stdout
    )
    writer = asyncio.StreamWriter(w_transport, w_protocol, reader, loop)
    return reader, writer


class JLinkRTT:
    def __init__(self, speed=None):
        # Clear DISPLAY to prevent the J-Link DLL from opening windows
        os.environ["DISPLAY"] = ""

        # Load the J-Link DLL, which needs to be installed on the host
        self.jlink = jlinksdk.JLink()

        # Make sure that J-Link will not pop up GUI windows
        self.jlink.EnableLog(None)
        self.jlink.SetWarnOutHandler(None)
        self.jlink.SetErrorOutHandler(None)

        self.up_buffers: list = None
        self.down_buffers: list = None
        if speed is None:
            speed = config.get_speed()
        self.speed = speed

    def _sync_connect_to_daemon(self):
        # Establish TCP connection to J-Link daemon via 127.0.0.1:19020
        try:
            self.jlink.Open(HostIF=jlinksdk.HOST_IF.TCPIP, sIP="127.0.0.1")
        except JLinkError as e:
            print("Can not connect to JLink, attempt retry")
            self.jlink.Open(HostIF=jlinksdk.HOST_IF.TCPIP, sIP="127.0.0.1")

    def _sync_connect_to_target(self):
        print(f"Connect JLink with speed {self.speed}")
        # Tell the J-Link daemon what kind of device we're talking to, and how to talk to it
        self.jlink.Connect(
            sDevice="VA416xx", TargetIF=jlinksdk.TIF.SWD, TIFSpeed=self.speed
        )

        # Find the SEGGER RTT block
        self.jlink.RTTerminal.Start(SEGGER_RTT_BLOCK_ADDRESS)

    def _sync_find_buffers(self):
        # Since there is no scanning for the RTT block, it should be found promptly.
        time.sleep(0.4)

        # Make sure the buffers were found
        num_up_buffers = self.jlink.RTTerminal.GetNumBuf(
            self.jlink.RTTerminal.BUFFER_DIR.UP
        )
        num_down_buffers = self.jlink.RTTerminal.GetNumBuf(
            self.jlink.RTTerminal.BUFFER_DIR.DOWN
        )
        if num_up_buffers is None or num_down_buffers is None:
            print(
                f"Could not find RTT buffer(s) at address {SEGGER_RTT_BLOCK_ADDRESS:#08x}"
                f"NumUpBuf = {num_up_buffers!r}, NumDownBuf = {num_down_buffers!r}",
                file=sys.stderr,
            )
            sys.exit(1)

        self.up_buffers = [
            self.jlink.RTTerminal.GetDesc(i, self.jlink.RTTerminal.BUFFER_DIR.UP)
            for i in range(num_up_buffers)
        ]
        self.down_buffers = [
            self.jlink.RTTerminal.GetDesc(i, self.jlink.RTTerminal.BUFFER_DIR.DOWN)
            for i in range(num_down_buffers)
        ]

    def _sync_disconnect_from_target(self):
        # FIXME: Do we care about the return value?
        self.jlink.RTTerminal.Stop()

    def _sync_disconnect_from_daemon(self):
        # FIXME: Do we care about the return value?
        self.jlink.Close()

    def open(self):
        assert not self.up_buffers and not self.down_buffers

        self._sync_connect_to_daemon()
        ok = False
        try:
            self._sync_connect_to_target()
            try:
                self._sync_find_buffers()
                ok = True
            finally:
                if not ok:
                    self._sync_disconnect_from_target()
        finally:
            if not ok:
                self._sync_disconnect_from_daemon()

    def close(self):
        self._sync_disconnect_from_target()
        self._sync_disconnect_from_daemon()
        self.up_buffers = self.down_buffers = None

    async def __aenter__(self):
        await asyncio.to_thread(self.open)
        return self

    async def __aexit__(self, exc_type, exc_value, traceback):
        await asyncio.to_thread(self.close)

    def stream_nonblocking(self, buffer_index: int) -> "JLinkRTTStreamNonblocking":
        assert buffer_index < len(self.up_buffers) and buffer_index < len(
            self.down_buffers
        ), (buffer_index, len(self.up_buffers), len(self.down_buffers))
        return JLinkRTTStreamNonblocking(self.jlink.RTTerminal, buffer_index)

    def stream_blocking(self, buffer_index: int) -> "JLinkRTTStreamBlocking":
        return JLinkRTTStreamBlocking(self.stream_nonblocking(buffer_index))

    def stream_async(self, buffer_index: int) -> "JLinkRTTStreamAsync":
        return JLinkRTTStreamAsync(self.stream_blocking(buffer_index))


class JLinkRTTStreamNonblocking:
    def __init__(self, rtt: jlinksdk.jlink._RTTERMINAL_API, buffer_index: int):
        self.rtt = rtt
        self.buffer_index = buffer_index

    closed = False

    def readable(self) -> bool:
        return True

    def read(self, max_bytes: int) -> bytes:
        # Non-blocking: we will return None if no data is available
        return self.rtt.Read(self.buffer_index, max_bytes) or None

    def write(self, data: bytes):
        # Non-blocking: we will return the actual number of bytes written, or None if we cannot write
        written = self.rtt.Write(self.buffer_index, data)
        assert 0 <= written <= len(data)
        return written or None


class JLinkRTTStreamBlocking:
    def __init__(self, nb_stream: JLinkRTTStreamNonblocking):
        self.nb_stream = nb_stream

    closed = False

    def readable(self) -> bool:
        return True

    def readinto(self, bytes_out: bytearray) -> int:
        bytes_read = self.read(len(bytes_out))
        if bytes_read is None:
            return None
        bytes_out[0 : len(bytes_read)] = bytes_read
        return len(bytes_read)

    def read(self, max_bytes: int, timeout_at=None) -> bytes:
        while True:
            bytes_read = self.nb_stream.read(max_bytes)
            if bytes_read is not None:
                return bytes_read
            if timeout_at is not None and time.time() > timeout_at:
                return None
            # Don't return until we have something, or else it will be treated as EOF
            time.sleep(0.005)

    def write(self, data: bytes):
        total_len = len(data)
        while data:
            num_written = self.nb_stream.write(data)
            if num_written is None:
                time.sleep(0.1)
            else:
                assert 1 <= num_written <= len(data)
                data = data[num_written:]
        return total_len


class JLinkRTTStreamAsync:
    def __init__(self, blocking_stream: JLinkRTTStreamBlocking):
        self.blocking_stream = blocking_stream
        self.write_buffer = []

    async def read(self, max_bytes: int) -> bytes:
        return await asyncio.to_thread(self.blocking_stream.read, max_bytes)

    def write(self, data: bytes):
        if data:
            self.write_buffer.append(data)

    async def drain(self):
        for data in self.write_buffer:
            await asyncio.to_thread(self.blocking_stream.write, data)
        del self.write_buffer[:]


async def main_async():

    # what and how to startup
    parser = argparse.ArgumentParser(
        description="Reads serial output/writes serial input via the JLink RTT",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-t",
        "--timestamps",
        dest="include_timestamp",
        action="store_true",
        help="Include a timestamp on each new line",
    )
    # NOTE: If time format was specified via environment variable, it'd be easier to users to even when
    # NOTE: using a testbed startup script that doesn't expose the terminal command line
    parser.add_argument(
        "-T",
        "--time-format",
        dest="time_fmt",
        default=DEFAULT_TIME_FMT,
        help="Specify timestamp format (in the syntax used by datetime.strftime())",
    )
    parser.add_argument(
        "-o",
        "--output",
        help="Path to file in which stdout should be stored (instead of terminal)",
    )
    parser.add_argument(
        "-s",
        "--speed",
        help=f"JLink speed (if not provided, {config.get_speed_src()}",
        default=config.get_speed(),
    )
    args = parser.parse_args()
    time_fmt = args.time_fmt if args.include_timestamp else None

    # get stdin & stdout
    stdin, stdout = await connect_stdin_stdout()
    async with OutputStreamWriter(stdout, args.output, time_fmt) as stdout_fd:

        async with JLinkRTT(args.speed) as rtt:
            print(
                f"Found {len(rtt.up_buffers)} up buffers and {len(rtt.down_buffers)} down buffers:"
            )
            for buffer_index, up_buffer in enumerate(rtt.up_buffers):
                if up_buffer.SizeOfBuffer:
                    print(
                        f"   Up Buffer {buffer_index} is named {up_buffer.sName} with size {up_buffer.SizeOfBuffer} and flags {up_buffer.Flags}"
                    )
            for buffer_index, down_buffer in enumerate(rtt.down_buffers):
                if down_buffer.SizeOfBuffer:
                    print(
                        f" Down Buffer {buffer_index} is named {down_buffer.sName} with size {down_buffer.SizeOfBuffer} and flags {down_buffer.Flags}"
                    )

            target_stdio = rtt.stream_async(0)
            await stdout_fd.awrite(b"\n\n\n**** TERMINAL CONNECTED ****\n")
            await asyncio.wait(
                [
                    asyncio.create_task(copy_stream_to_stream(target_stdio, stdout_fd)),
                    asyncio.create_task(copy_stream_to_stream(stdin, target_stdio)),
                ]
            )


def main():
    try:
        asyncio.run(main_async())
    except (KeyboardInterrupt, RuntimeError):
        print("\n --- Terminated by Ctrl + C ---\n")


if __name__ == "__main__":
    main()
