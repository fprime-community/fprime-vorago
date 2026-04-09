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
from typing import List, Optional, Tuple


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
            "-l",  # Source code line numbers
            "-d",  # Decode only .text
            filename,
        ],
        stdout=subprocess.PIPE,
        text=True,
    )

    assert process.stdout, "Failed to pipe stdout"

    errors: List[Tuple[str, Optional[str], Optional[str], Optional[str]]] = []

    # Clear lines until we get to the .text section
    for line in process.stdout:
        if "Disassembly of section .text" in line:
            break

    # Symbol names are set in the .text by the linker
    # These are the most robust labels in the final binary
    symbol_name = None

    # Function names are mapped via DWARF symbols and are typically per-instruction
    function_name = None

    # Location is the line number and file location this instruction corresponds to
    # This information is also found in the DWARF symbols
    location = None

    illegal_instructions = ("strb", "strexb", "ldrexb")

    for line in process.stdout:
        # This is an instruction
        if line.startswith("    "):
            if "__badstrb_strb" in line:
                continue

            # Rule out all instructions within the whitelist
            if (
                function_name is None and symbol_name in whitelist
            ) or function_name in whitelist:
                continue

            for i in illegal_instructions:
                if i in line:
                    errors.append((i, function_name, symbol_name, location))

        elif line.endswith(">:\n"):
            symbol_name = line[line.find("<") + 1 : -3]
        elif line.endswith("():\n"):
            function_name = line[: line.find("():")]
        else:
            line = line.strip()
            if line:
                # All other non-empty lines are source locations
                location = line.strip()
            else:
                # Empty lines mark the end of the function/symbol
                symbol_name = None
                function_name = None
                location = None

    exit_code = process.wait()
    assert exit_code == 0, "objdump failed"

    if errors:
        print(
            red("ERROR")
            + f" Illegal 8-bit operations detected in {target_name} with badstrb feature:",
            file=sys.stderr,
        )
        for instruction, function_name, symbol_name, location in errors:
            print(red("ERROR") + " Instruction " + red(instruction), file=sys.stderr)
            print(f"INFO     Function '{function_name}'", file=sys.stderr)
            print(f"INFO     Symbol '{symbol_name}'", file=sys.stderr)
            print(f"INFO     Location '{location}'", file=sys.stderr)
        print(f"INFO Function whitelist:", file=sys.stderr)
        for item in whitelist:
            print(f"INFO     {item}", file=sys.stderr)
        exit(1)


if __name__ == "__main__":
    main()
