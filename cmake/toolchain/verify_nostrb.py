#!/usr/bin/python3
# Copyright 2026 California Institute of Technology
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
import subprocess
import sys
import os
import typing


def red(sub: str):
    return f"\033[31m{sub}\033[0m"


def main():
    parser = argparse.ArgumentParser(
        prog="verify_nostrb",
        description="Verify there are no illegal strb or strexb/ldrexb instructions in a ELF binary",
    )

    parser.add_argument("filename", help="ELF binary to verify")
    parser.add_argument(
        "--whitelist",
        action="store",
        help="Ignore a set of functions/labels in the binary, delimited with ';'",
    )
    parser.add_argument(
        "--name",
    )

    args = parser.parse_args()

    whitelist = set(args.whitelist.split(";"))
    filename = args.filename
    target_name = args.name

    process = subprocess.Popen(
        [
            "arm-none-eabi-objdump",
            "-d",
            filename,
        ],
        stdout=subprocess.PIPE,
        text=True,
    )

    assert process.stdout, "Failed to pipe stdout"

    errors = []

    # Clear lines until we get to the .text section
    for line in process.stdout:
        if "Disassembly of section .text" in line:
            break

    for line in process.stdout:
        if line.endswith(">:\n"):
            # This is a function section
            func_name = line[line.find("<") + 1 : -3]

            for line in process.stdout:
                if line == "\n":
                    # Empty line marks the end of the function
                    break

                if "__badstrb_strb" in line or func_name in whitelist:
                    continue

                if "strb" in line:
                    errors.append(f"Found {red('strb')} in '{func_name}'")

                if "strexb" in line:
                    errors.append(f"Found {red('strexb')} in '{func_name}'")

                if "ldrexb" in line:
                    errors.append(f"Found {red('ldrexb')} in '{func_name}'")

    exit_code = process.wait()
    assert exit_code == 0, "objdump failed"

    if errors:
        print(
            red("ERROR")
            + f" Illegal 8-bit operations detected in {target_name} with badstrb feature:",
            file=sys.stderr,
        )
        for err in errors:
            print(red("ERROR") + f"     {err}", file=sys.stderr)
        print(f"INFO: Function whitelist:", file=sys.stderr)
        for item in whitelist:
            print(f"INFO:     {item}", file=sys.stderr)
        exit(1)


if __name__ == "__main__":
    main()
