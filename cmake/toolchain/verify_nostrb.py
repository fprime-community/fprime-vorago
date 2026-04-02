#!/usr/bin/python3
import sys
import os

whitelist_raw = os.environ.get("VERIFY_NO_STRB_WHITELIST", "")
whitelist = whitelist_raw.split(";")
target_name = os.environ.get("TARGET_NAME", "<unknown>")

# Clear lines until we get to the .text section
for line in sys.stdin:
    if "Disassembly of section .text" in line:
        break

errors = []

def red(sub: str):
    return f"\033[31m{sub}\033[0m"

for line in sys.stdin:
    if line.endswith(">:\n"):
        # This is a function section
        func_name = line[line.find("<") + 1:-3]

        for line in sys.stdin:
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

if errors:
    print(red("ERROR") + f" Invalid 8-bit operations in {target_name} detected with badstrb feature:", file=sys.stderr)
    for err in errors:
        print(red("ERROR") + f"     {err}", file=sys.stderr)
    print(f"INFO: Function whitelist:", file=sys.stderr)
    for item in whitelist:
        print(f"INFO:     {item}", file=sys.stderr)
    exit(1)
