# Question: do I need to include a lisence here?

import argparse
import pathlib
import struct
import sys
import zlib
from typing import Optional

# Fill value for padding
PAD_BYTE = b"\xff"

# Number of bytes each image check reserves at the end of the image region.
# Keep in sync with IMAGE_CHECK_SIZE in register_with_bsp (va416x0-baremetal.cmake).
IMAGE_CHECK_SIZES = {
    "crc32": 4,
}


def parse_int(value: str) -> int:
    # Accepts decimal or 0x-prefixed hexadecimal, as passed from CMake
    return int(value, 0)


def stamp_image(image: bytes, image_check: Optional[str], region_size: int) -> bytes:
    image_check = str(image_check).lower()
    check_size = IMAGE_CHECK_SIZES.get(image_check, 0)
    content_size = region_size - check_size
    if len(image) > content_size:
        sys.exit(
            f"ERROR: image is {len(image)} bytes, but only {content_size} bytes are available."
        )

    # Left pad binary, and then add on the crc32
    padded = image.ljust(content_size, PAD_BYTE) if check_size else image
    value = b""
    if image_check == "crc32":
        value = struct.pack("<I", zlib.crc32(padded))

    if len(value) != check_size:
        sys.exit(
            f"ERROR: {image_check} value size isn't needed {check_size} bytes. Got {len(value)} bytes."
        )
    if len(value):
        return padded + value
    return image  # avoid reallocating


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Stamp an image check into a raw binary. Optionally prepend a bootloader."
    )
    parser.add_argument(
        "--image-check", type=str, help="Kind of image check to stamp into the image"
    )
    parser.add_argument(
        "--image-start",
        required=True,
        type=parse_int,
        help="CODE_SRAM address the input binary is linked at",
    )
    parser.add_argument(
        "--image-end",
        required=True,
        type=parse_int,
        help="End (exclusive) of the image's CODE_SRAM region",
    )
    parser.add_argument(
        "--bootloader",
        type=pathlib.Path,
        help="Raw bootloader binary (linked at address 0) to prepend, padded up to --image-start",
    )
    parser.add_argument("input", type=pathlib.Path, help="Raw image binary")
    parser.add_argument("output", type=pathlib.Path, help="Output binary")

    args = parser.parse_args()

    end = args.image_end
    start = args.image_start
    region_size = end - start
    if region_size <= 0:
        sys.exit(
            f"ERROR: image end (0x{end:X}) must be greater than image start (0x{start:X})"
        )

    try:
        image = args.input.read_bytes()
    except OSError as e:
        sys.exit(f"ERROR: failed to read image '{args.input}': {e}")

    output = stamp_image(image, args.image_check, region_size)

    if args.bootloader is not None:
        try:
            bootloader = args.bootloader.read_bytes()
        except OSError as e:
            sys.exit(f"ERROR: failed to read bootloader '{args.bootloader}': {e}")
        if len(bootloader) > start:
            sys.exit(
                f"ERROR: bootloader is {len(bootloader)} bytes, which overlaps the image at 0x{start:X}"
            )
        output = bootloader.ljust(start, PAD_BYTE) + output

    # Question: should I then left pad by bytes if there is no bootloader?

    try:
        args.output.write_bytes(output)
    except OSError as e:
        sys.exit(f"ERROR: failed to write '{args.output}': {e}")
