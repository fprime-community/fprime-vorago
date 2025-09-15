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
"""
Module for getting common JLink configuration
"""
import os

# Environment variable for default Jlink connection speed
SPEED_ENV = "JLINK_DEFAULT_SPEED"


def get_speed():
    return os.environ.get(SPEED_ENV, 2000)


def get_speed_src():
    return f"set by environment variable {SPEED_ENV} if set, else set to a hard coded value"


# FIXME: Consider providing gets for all common config
# def get_device():
#     return "VA416xx"

# def get_ip():
#     return "127.0.0.1"

# def get_interface_type():
#     return "SWD"
