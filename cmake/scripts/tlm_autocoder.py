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

###############################################################################
# cmake/scripts/tlm_autocoder.py
###############################################################################

import argparse
import collections
import functools
import pathlib
import sys
import typing

import jinja2

from fprime_gds.common.models.dictionaries import Dictionaries
from fprime_gds.common.models.serialize.numerical_types import NumericalType
from fprime_gds.common.models.serialize.string_type import StringType
from fprime_gds.common.models.serialize.type_base import BaseType
from fprime_gds.common.templates.ch_template import ChTemplate
from fprime_gds.common.utils.config_manager import ConfigManager

TEMPLATE_PATH = pathlib.Path(__file__).resolve().parent.parent / "templates"

# Written to the stub file for deployments that do not include TlmGdsChan
STUB_FILE_CONTENTS = """\
/*
** This is a stub file produced by the telemetry autocoder. This deployment
** does not contain the Va416x0.TlmGdsChan component so the telemetry
** configuration tables will not be generated.
*/
"""


# NOTE: get_constant, get_typedef, and get_type_size are vendored from
# tools/common/dictionary.py so that this script has no project-specific dependency.


@functools.cache
def get_constant(dictionary: Dictionaries, name: str) -> int:
    """
    Get the value of an FPP constant from the given F-Prime dictionary.

    :param dictionary: F-Prime dictionary
    :param name: Constant name
    :return: the value of the constant
    """
    assert dictionary.constant_name is not None
    if name not in dictionary.constant_name:
        raise ValueError(f"Failed to find a constant definition matching {name}")
    return dictionary.constant_name[name]


def get_typedef(dictionary: Dictionaries, name: str) -> type[BaseType]:
    """
    Get the type object for the given type name from the F-Prime dictionary.

    :param dictionary: F-Prime dictionary
    :param name: Type name
    """
    assert dictionary.typedefs_name is not None
    if name not in dictionary.typedefs_name:
        raise ValueError(f"Failed to find a type definition matching {name}")
    return dictionary.typedefs_name[name]


def get_type_size(
    dictionary: Dictionaries,
    name: str | None = None,
    type_obj: typing.Any | None = None,
) -> int:
    """
    Derive the size of an F-Prime type using the given dictionary. Either the
    type name or a type object can be provided.

    :param dictionary: F-Prime dictionary
    :param name: Type name
    :param type_obj: Type object
    :return: the size of the type, in bytes
    """
    if name is not None:
        type_obj = get_typedef(dictionary, name)()
    if type_obj is None:
        raise ValueError("get_type_size: Missing name or type_obj")

    # NOTE: use FW_TLM_STRING_MAX_SIZE plus sizeof(FwSizeStoreType) to size string channels
    # When a string is serialized, ConstStringBase prepends the length as an FwSizeStoreType (see Fw/Types/ConstStringBase.cpp)
    if isinstance(type_obj, StringType):
        size_store_type = ConfigManager().get_type("FwSizeStoreType")
        assert issubclass(size_store_type, NumericalType)
        size = (
            get_constant(dictionary, "FW_TLM_STRING_MAX_SIZE")
            + size_store_type.getSize()
        )
    else:
        size = type_obj.getMaxSize()

    return size


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=f"Auto-codes telemetry configuration files for the given deployment."
    )
    parser.add_argument("deployment")
    parser.add_argument("dictionary_path", type=pathlib.Path)
    parser.add_argument("output_path", type=pathlib.Path)
    return parser.parse_args()


def partition_component_channels(
    dictionary: Dictionaries,
) -> dict[int, list[ChTemplate]]:
    components = collections.defaultdict(list)
    component_mask = get_constant(dictionary, "Va416x0.ComponentIdMask")
    component_shift = get_constant(dictionary, "Va416x0.ComponentIdOffset")

    # Sort channels by ID
    assert dictionary.channel_id is not None
    for channel_id, channel in sorted(dictionary.channel_id.items()):
        component_id = (channel_id & component_mask) >> component_shift
        components[component_id].append(channel)

    return components


def telemetry_entry_size(dictionary: Dictionaries, channel: ChTemplate) -> int:
    name = channel.get_full_name()
    size = get_type_size(dictionary, type_obj=channel.ch_type_obj())

    # Verify that the channel value can be fully indexed by the length field
    length_type_size = get_type_size(
        dictionary, name="Va416x0.TlmGdsChan.TlmItemLengthType"
    )
    max_length = (1 << (length_type_size * 8)) - 1
    if size > max_length:
        sys.exit(
            f"ERROR: Channel {name} has size {size} bytes which is larger than Va416x0.TlmGdsChan "
            f"can support, maximum allowed size is {max_length} bytes"
        )

    # Each telemetry item is stored with:
    # (1) the channel ID (U16 == 2 bytes)
    # (2) the sequence number (U8 == 1 byte)
    # (3) the length of the channel value (U8 == 1 byte)
    # (4) the timestamp seconds value (U32 == 4 bytes)
    # (5) the timestamp microseconds value (U32 == 4 bytes)
    # (6) the channel data (variable length; for strings the maximum is FW_TLM_STRING_MAX_SIZE plus
    #     the serialized length prefix, see get_type_size)
    # NOTE: this must be kept up-to-date with the struct definition of TlmGdsChan::TlmItemHeader
    # inside Components/Va416x0/TlmGdsChan/TlmGdsChan.hpp
    chan_id_size = get_type_size(dictionary, name="FwChanIdType")
    sequence_size = get_type_size(
        dictionary, name="Va416x0.TlmGdsChan.TlmItemSequenceType"
    )
    timestamp_size = 8
    full_size = chan_id_size + sequence_size + length_type_size + timestamp_size + size
    # Entries will be U32-aligned (updated from U16-aligned in v2)
    remainder = full_size % 4
    if remainder != 0:
        full_size += 4 - remainder

    return full_size


def validate_tlm_gds_chan_config(
    dictionary: Dictionaries, channels: dict[int, list[ChTemplate]]
):
    # Count the total memory size needed to store all channel values
    total_size = 2  # U16 magic word precedes the buffer
    for component_channels in channels.values():
        for channel in component_channels:
            total_size += telemetry_entry_size(dictionary, channel)

    buffer_size_constant = "Va416x0.TlmGdsChanCfg.PingPongBufferSize"
    buffer_size = get_constant(dictionary, buffer_size_constant)
    if buffer_size < total_size:
        sys.exit(
            f"ERROR: {buffer_size_constant} is set to {buffer_size} bytes but {args.deployment} "
            f"channels require a buffer size of at least {total_size} bytes, raise the value of "
            f"{buffer_size_constant} via CONFIGURATION_OVERRIDES and retry"
        )


def create_channel_table(
    dictionary: Dictionaries, channels: dict[int, list[ChTemplate]]
) -> list[dict]:
    table = []
    component_shift = get_constant(dictionary, "Va416x0.ComponentIdOffset")

    for component_id, component_channels in channels.items():
        # Add array elements for channels, accounting for gaps in channel IDs
        id = 0
        for channel in component_channels:
            # Fill in any gaps in IDs
            while channel.id != ((component_id << component_shift) | id):
                table.append(
                    {
                        "size": 0,
                        "id": (component_id << component_shift) | id,
                        "name": "UNUSED",
                    }
                )
                id += 1
            size = get_type_size(dictionary, type_obj=channel.ch_type_obj())
            table.append(
                {
                    "size": size,
                    "id": channel.id,
                    "name": channel.get_full_name(),
                }
            )
            id += 1

    return table


def create_lookup_table(channels: dict[int, list[ChTemplate]]) -> list[dict]:
    table = []

    # Get the total number of components, accounting for any gaps in component IDs
    component_count = list(channels.keys())[-1] + 1 if channels else 0

    offset = 0
    for id in range(component_count):
        count = len(channels.get(id, []))
        table.append(
            {
                "id": id,
                "count": count,
                "offset": offset,
            }
        )
        offset += count

    return table


def main(args: argparse.Namespace):
    dictionary = Dictionaries()
    dictionary.load_dictionaries(str(args.dictionary_path), None, None)

    # Check if TlmGdsChan constants are found in the dictionary. If they are not, just generate a
    # stub file and bail out
    try:
        get_constant(dictionary, "Va416x0.TlmGdsChanCfg.PingPongBufferSize")
        get_type_size(dictionary, name="Va416x0.TlmGdsChan.TlmItemSequenceType")
        get_type_size(dictionary, name="Va416x0.TlmGdsChan.TlmItemLengthType")
    except ValueError:
        args.output_path.write_text(STUB_FILE_CONTENTS)
        sys.exit(0)

    # Grab telemetry channels from the dictionary, sorting and splitting by component
    channels = partition_component_channels(dictionary)

    # Count the total number of channels, accounting for any gaps in channel IDs
    channel_count = 0
    channel_mask = get_constant(dictionary, "Va416x0.DictionaryItemIdMask")
    channel_shift = get_constant(dictionary, "Va416x0.DictionaryItemIdOffset")
    for component_channels in channels.values():
        # Channels are sorted by ID, the final channel will give us the count inclusive of any gaps
        final_id = (component_channels[-1].id & channel_mask) >> channel_shift
        channel_count += final_id + 1

    # Count the total number of components, accounting for any gaps in component IDs
    component_count = list(channels.keys())[-1] + 1 if channels else 0

    # Validate that the TlmGdsChan configuration can fit the channels
    validate_tlm_gds_chan_config(dictionary, channels)

    # Create the channel table
    channel_table = create_channel_table(dictionary, channels)
    # Create the component channel lookup table
    component_lookup_table = create_lookup_table(channels)

    # Render the template
    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(TEMPLATE_PATH),
        trim_blocks=True,
        lstrip_blocks=True,
    )
    template = env.get_template("TlmCfg.cpp.jinja")
    output = template.render(
        filename=args.output_path.name,
        script=pathlib.Path(__file__).name,
        deployment=args.deployment,
        channel_count=channel_count,
        component_count=component_count,
        channel_table=channel_table,
        component_lookup_table=component_lookup_table,
    )
    # Write to the output file
    args.output_path.write_text(output.rstrip() + "\n")


if __name__ == "__main__":
    try:
        args = parse_args()
        main(args)
    except Exception as ex:
        sys.exit(f"ERROR: {ex}")
