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

"""
gds_plugin.py:

This adapter allows projects to interface the F Prime GDS with an embedded system using a virtual serial port tunneled
using the SEGGER RTT (Real-Time-Terminal) protocol over a J-Link debugging probe.
"""

import io
import logging
import threading
import time

from fprime_gds.common.communication.adapters.base import BaseAdapter
from fprime_gds.plugin.definitions import gds_plugin_implementation
from Va416x0.Os.SeggerTerminal.terminal import JLinkRTT
from pylink.errors import JLinkException as JLinkError

LOGGER = logging.getLogger("segger_rtt_adapter")


class SeggerRttAdapter(BaseAdapter):
    def __init__(self):
        self.rtt = JLinkRTT()
        self.stdio_stream = None
        self.gds_stream = None
        self.stdout_thread = None
        self.read_buffer_size = None

    def __repr__(self):
        """String representation for logging"""
        return f"SEGGER-RTT@127.0.0.1"

    def stdout_thread_main(self):
        reader = io.BufferedReader(self.stdio_stream)
        try:
            while True:
                line = reader.readline()
                if not line:
                    break
                LOGGER.debug(
                    "Target: %s", line.rstrip(b"\n").decode("utf-8", "replace")
                )
        except JLinkError:
            LOGGER.info("STDOUT stream from target terminated due to JLinkError")

    def open(self):
        self.close()
        assert self.stdout_thread is None

        LOGGER.info("Opening SEGGER RTT connection...")
        try:
            self.rtt.open()
            LOGGER.info("SEGGER RTT connection opened successfully")
        except Exception as e:
            LOGGER.error("Failed to open SEGGER RTT connection: %s", e)
            raise

        if not self.rtt.up_buffers or not self.rtt.down_buffers:
            LOGGER.error("RTT buffers not found after open()")
            raise RuntimeError("Failed to find RTT buffers")

        LOGGER.info(
            "Found %d up buffers and %d down buffers",
            len(self.rtt.up_buffers),
            len(self.rtt.down_buffers),
        )
        for buffer_index, up_buffer in enumerate(self.rtt.up_buffers):
            if up_buffer.SizeOfBuffer:
                LOGGER.debug(
                    "   Up Buffer %d is named %s with size %d and flags %d",
                    buffer_index,
                    up_buffer.name,
                    up_buffer.SizeOfBuffer,
                    up_buffer.Flags,
                )
        for buffer_index, down_buffer in enumerate(self.rtt.down_buffers):
            if down_buffer.SizeOfBuffer:
                LOGGER.debug(
                    " Down Buffer %d is named %s with size %d and flags %d",
                    buffer_index,
                    down_buffer.name,
                    down_buffer.SizeOfBuffer,
                    down_buffer.Flags,
                )

        LOGGER.debug("Creating stream for buffer 0 (stdio)")
        self.stdio_stream = self.rtt.stream_blocking(0)
        LOGGER.debug("Creating stream for buffer 1 (gds)")
        self.gds_stream = self.rtt.stream_blocking(1)
        self.read_buffer_size = self.rtt.up_buffers[1].SizeOfBuffer
        LOGGER.info("GDS stream read buffer size: %d", self.read_buffer_size)
        assert self.read_buffer_size >= 16

        self.stdout_thread = threading.Thread(
            target=self.stdout_thread_main, daemon=True
        )
        self.stdout_thread.start()
        LOGGER.info("SEGGER RTT adapter fully initialized and ready")

    def close(self):
        if self.stdout_thread is None:
            return
        try:
            LOGGER.debug("closing SEGGER RTT connection")
            self.rtt.close()
            self.stdout_thread.join(timeout=1)
            # Not safe to proceed until this thread is dead...
            if self.stdout_thread.is_alive():
                LOGGER.critical("logger thread does not appear to be terminating...?")
                self.stdout_thread.join()
            LOGGER.debug("joined logger thread")
        finally:
            self.stdout_thread = None
            self.stdio_stream = None
            self.gds_stream = None
            self.read_buffer_size = None

    def write(self, frame):
        """
        Send a given framed bit of data by sending it out the serial interface. It will attempt to reconnect if there is
        was a problem previously. This function will return true on success, or false on error.

        :param frame: framed data packet to send out
        :return: True, when data was sent through the UART. False otherwise.
        """
        if self.stdout_thread is None:
            self.open()
        try:
            self.gds_stream.write(frame)
            return True
        except JLinkError as e:
            LOGGER.error("J-Link write failed: %s", e)
            self.close()
        return False

    def read(self, timeout=0.5):
        if self.stdout_thread is None:
            self.open()
        try:
            data = self.gds_stream.read(
                self.read_buffer_size, timeout_at=time.time() + timeout
            )
            if data:
                LOGGER.debug("Read %d bytes from GDS stream", len(data))
                return data
            else:
                LOGGER.debug("No data available from GDS stream (timeout)")
        except JLinkError as e:
            LOGGER.error("J-Link read failed: %s", e)
            self.close()
        return b""

    @classmethod
    def get_arguments(cls):
        """
        Returns a dictionary of flag to argparse-argument dictionaries for use with argparse to setup arguments.

        :return: dictionary of flag to argparse arguments for use with argparse
        """
        return {}

    @classmethod
    def get_name(cls):
        """Get name of the adapter"""
        return "segger_rtt"

    @classmethod
    @gds_plugin_implementation
    def register_communication_plugin(cls):
        """Register this as a communication plugin"""
        return cls

    @classmethod
    def check_arguments(cls):
        """
        Code that should check arguments of this adapter. If there is a problem with this code, then a "ValueError"
        should be raised describing the problem with these arguments.
        """
        # No arguments
        pass
