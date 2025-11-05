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
from jlinksdk import JLinkError

LOGGER = logging.getLogger("segger_rtt_adapter")
# FIXME: Get rid of this once https://github.com/nasa/fprime/issues/3561 is implemented
LOGGER.setLevel(logging.DEBUG)


class SeggerRttAdapter(BaseAdapter):
    def __init__(self):
        self.rtt = JLinkRTT()
        self.stdio_stream = None
        self.gds_stream = None
        self.stdout_thread = None
        self.read_buffer_size = None

    def __repr__(self):
        """ String representation for logging """
        return f"SEGGER-RTT@127.0.0.1"

    def stdout_thread_main(self):
        reader = io.BufferedReader(self.stdio_stream)
        try:
            while True:
                line = reader.readline()
                if not line:
                    break
                LOGGER.debug("Target: %s", line.rstrip(b"\n").decode('utf-8', 'replace'))
        except JLinkError:
            LOGGER.info("STDOUT stream from target terminated due to JLinkError")

    def open(self):
        self.close()
        assert self.stdout_thread is None

        LOGGER.info("Opening SEGGER RTT connection...")
        self.rtt.open()

        LOGGER.debug("%s", f"Found {len(self.rtt.up_buffers)} up buffers and {len(self.rtt.down_buffers)} down buffers:")
        for buffer_index, up_buffer in enumerate(self.rtt.up_buffers):
            if up_buffer.SizeOfBuffer:
                LOGGER.debug("%s", f"   Up Buffer {buffer_index} is named {up_buffer.sName} with size {up_buffer.SizeOfBuffer} and flags {up_buffer.Flags}")
        for buffer_index, down_buffer in enumerate(self.rtt.down_buffers):
            if down_buffer.SizeOfBuffer:
                LOGGER.debug("%s", f" Down Buffer {buffer_index} is named {down_buffer.sName} with size {down_buffer.SizeOfBuffer} and flags {down_buffer.Flags}")

        self.stdio_stream = self.rtt.stream_blocking(0)
        self.gds_stream = self.rtt.stream_blocking(1)
        self.read_buffer_size = self.rtt.up_buffers[1].SizeOfBuffer
        assert self.read_buffer_size >= 16

        self.stdout_thread = threading.Thread(target=self.stdout_thread_main, daemon=True)
        self.stdout_thread.start()

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
            data = self.gds_stream.read(self.read_buffer_size, timeout_at=time.time() + timeout)
            if data:
                return data
        except JLinkError as e:
            LOGGER.error("J-Link read failed: %s", e)
            self.close()
        
        # This condition was happening due to the cpu reset
        # perhaps better to check if the jlink is connected
        # Saw ARM DISCONNECT or something similar in the logfile
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
        """ Get name of the adapter """
        return "segger_rtt"

    @classmethod
    @gds_plugin_implementation
    def register_communication_plugin(cls):
        """ Register this as a communication plugin """
        return cls

    @classmethod
    def check_arguments(cls):
        """
        Code that should check arguments of this adapter. If there is a problem with this code, then a "ValueError"
        should be raised describing the problem with these arguments.
        """
        # No arguments
        pass
