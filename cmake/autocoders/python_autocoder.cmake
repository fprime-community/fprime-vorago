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

autocoder_setup_for_individual_sources()

function(python_autocoder_is_supported AC_INPUT_FILE)
    autocoder_support_by_suffix(".py" "${AC_INPUT_FILE}")
endfunction(python_autocoder_is_supported)

function(python_autocoder_setup_autocode BUILD_TARGET_NAME AC_INPUT_FILE)
    # Get the name without the directory
    get_filename_component(BASENAME "${AC_INPUT_FILE}" NAME)
    # Strip off the last part of the filename
    string(REGEX REPLACE "\\.[^.]+$" "" BASENAME_SHORT ${BASENAME})
    set(PYTHON_OUTPUT_CPP "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME_SHORT}Ac.cpp")
    set(AUTOCODER_GENERATED_BUILD_SOURCES "${PYTHON_OUTPUT_CPP}" PARENT_SCOPE)

    add_custom_command(
        OUTPUT "${PYTHON_OUTPUT_CPP}"
        COMMAND "${PYTHON}" "${AC_INPUT_FILE}" >"${CMAKE_CURRENT_BINARY_DIR}/${BASENAME_SHORT}Ac.cpp"
        DEPENDS "${AC_INPUT_FILE}"
    )

    set(AUTOCODER_BUILD_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME_SHORT}Ac.cpp" PARENT_SCOPE)
endfunction()
