#!/usr/bin/env python3
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
import pathlib
import struct
import subprocess
import sys
import tempfile
import zlib
from typing import Optional

PAD_BYTE = b"\xff"
IMAGE_STAMPS = {
    "crc32": lambda data: struct.pack("<I", zlib.crc32(data)),  # little endian
}


def combine_image(
    bootloader: bytes,
    app: bytes,
    app_base: int,
    app_end: int,
    image_stamp: Optional[str],
) -> bytes:
    compute_check = IMAGE_STAMPS[image_stamp] if image_stamp else lambda _: b""
    check_size = len(compute_check(b""))
    if len(bootloader) > app_base:
        sys.exit(
            f"ERROR: bootloader length of {len(bootloader)} bytes overlaps the application at 0x{app_base:X}"
        )
    app_size = app_end - app_base - check_size
    if len(app) > app_size and (
        len(app) > app_end - app_base or app[app_size:].strip(PAD_BYTE)
    ):
        sys.exit(
            f"ERROR: application binary is {len(app)} bytes but only {app_size} bytes are available"
        )

    padded_app = app[:app_size].ljust(app_size, PAD_BYTE)
    return bootloader.ljust(app_base, PAD_BYTE) + padded_app + compute_check(padded_app)


def stamp_elf(elf: pathlib.Path, content: bytes):
    # Write the image check into the ELF's .image_stamp section (reserved by va416x0.ld)
    with tempfile.NamedTemporaryFile(suffix=".bin") as f:
        f.write(content)
        f.flush()
        subprocess.run(
            [
                "arm-none-eabi-objcopy",
                f"--update-section=.image_stamp={f.name}",
                str(elf),
            ],
            check=True,
        )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Combine a bootloader and application binary. Optionally stamp the application."
    )
    parser.add_argument(
        "--app-base",
        required=True,
        type=lambda v: int(v, 0),
        help="Start application instruction address",
    )
    parser.add_argument(
        "--app-end",
        required=True,
        type=lambda v: int(v, 0),
        help="End (exclusive) application instruction address",
    )
    parser.add_argument(
        "--image-stamp",
        required=False,
        choices=list(IMAGE_STAMPS.keys()) + [None],
        default=None,
        help="Image stamp type to stamp at the end of the application region",
    )
    parser.add_argument(
        "--elf",
        type=pathlib.Path,
        help="Application ELF to stamp the image check into",
    )
    parser.add_argument(
        "bootloader",
        type=pathlib.Path,
        help="Bootloader binary (linked at address 0x0)",
    )
    parser.add_argument(
        "app", type=pathlib.Path, help="Application binary (linked at --app-base)"
    )
    parser.add_argument("output", type=pathlib.Path, help="Combined output binary")
    args = parser.parse_args()

    if not 0 < args.app_base < args.app_end:
        sys.exit(
            f"ERROR: expected 0 < app base (0x{args.app_base:X}) < app end (0x{args.app_end:X})"
        )
    try:
        combined = combine_image(
            args.bootloader.read_bytes(),
            args.app.read_bytes(),
            args.app_base,
            args.app_end,
            args.image_stamp,
        )
        args.output.write_bytes(combined)
        stamp_size = len(IMAGE_STAMPS[args.image_stamp](b"")) if args.image_stamp else 0
        if args.elf and stamp_size:
            stamp_elf(args.elf, combined[len(combined) - stamp_size :])
    except (OSError, subprocess.CalledProcessError) as e:
        sys.exit(f"ERROR: {e}")
