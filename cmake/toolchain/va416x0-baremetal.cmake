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

set(CMAKE_SYSTEM_NAME Generic)
set(FPRIME_PLATFORM va416x0-baremetal)
set(CMAKE_SYSTEM_PROCESSOR armv7l)

# This toolchain file is intended to be used with the toolchain available from:
#   https://github.com/arm/arm-toolchain/tree/arm-software/arm-software/embedded
# Specifically release-20.1.0-ATfE
set(CMAKE_C_COMPILER clang-20)
set(CMAKE_CXX_COMPILER clang-20)
set(CMAKE_ASM_COMPILER clang-20)

# FIXME: This is only needed because our linker script and linker arguments
#  necessitate the use of symbols provided by Va416x0/Svc/VectorTable.
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_ASM_COMPILER_WORKS 1)

set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/va416x0.ld)

set(VA416X0_COMMON_FLAGS "\
    --target=thumbv7m-unknown-none-eabi \
    -mcpu=cortex-m4 \
    -mthumb \
    -ggdb3 \
    -mfpu=fpv4-sp-d16 \
    -mfloat-abi=hard \
")

set(VA416X0_COMMON_LTO_FLAGS "\
    ${VA416X0_COMMON_FLAGS} \
    -fstrict-vtable-pointers \
")

if (NOT DEFINED VA416X0_DISABLE_LTO)
    set(VA416X0_COMMON_LTO_FLAGS "\
        ${VA416X0_COMMON_LTO_FLAGS} \
        -flto \
        -fwhole-program-vtables \
    ")
endif()

set(VA416X0_COMMON_C_CXX_FLAGS "\
    ${VA416X0_COMMON_LTO_FLAGS} \
    -fno-exceptions \
    -fno-rtti \
    -ffunction-sections \
    -fdata-sections \
    -pedantic \
    -Wextra \
    -Wno-unused-parameter \
    -Werror \
    -fstack-usage \
")

if ((DEFINED VA416X0_ENABLE_PROFILER) AND VA416X0_ENABLE_PROFILER)
    # Functions can be excluded from instrumentation for the profiler either on a per-file basis
    # via -finstrument-functions-exclude-file-list or on a per-function (using de-mangled names)
    # basis via -finstrument-functions-exclude-function-list
    set(VA416X0_COMMON_C_CXX_FLAGS "\
        ${VA416X0_COMMON_C_CXX_FLAGS} \
        -finstrument-functions-after-inlining \
        -DVA416X0_ENABLE_PROFILER \
    ")
endif()

if (NOT DEFINED VA416X0_DISABLE_LTO)
    set(VA416X0_COMMON_C_CXX_FLAGS "\
        ${VA416X0_COMMON_C_CXX_FLAGS} \
        -Wframe-larger-than=1800 \
    ")
endif()

set(CMAKE_C_FLAGS "\
    ${VA416X0_COMMON_C_CXX_FLAGS} \
")

set(CMAKE_CXX_FLAGS "\
    ${VA416X0_COMMON_C_CXX_FLAGS} \
    -fno-use-cxa-atexit \
    -Wold-style-cast \
    -Wno-c++20-attribute-extensions \
")

set(CMAKE_ASM_FLAGS "\
    ${VA416X0_COMMON_FLAGS} \
    -x assembler-with-cpp \
    -DDEBUG \
")

# Note: --enable-non-contiguous-regions is a LLVM LLD 19.1.0 or newer feature.
# See https://discourse.llvm.org/t/lld-linker-section-packing/70234 for more
# context and explanation. We need it to safely use both 32 KiB memory regions
# for our .bss region without relying on manual packing.
#
# Note: -Wl,-z,norelro disables a security check that is NOT APPLICABLE to
# embedded environments, but which sometimes causes errors with linking the
# .tdata section due to discontinuities of different relro-enabled memory
# regions.
#
# The inclusion of --save-temps makes LLD save debugging information on the LTO
# process by default. This is not needed, but the overhead is less than half a
# second totaled across over a dozen deployments, so it seems reasonable for us
# to make it the standard.
set(CMAKE_EXE_LINKER_FLAGS "\
    ${VA416X0_COMMON_LTO_FLAGS} \
    -nostartfiles \
    -Wl,--gc-sections \
    -Wl,-z,norelro \
    -Wl,--lto-whole-program-visibility \
    -Wl,--enable-non-contiguous-regions \
    -static \
    -Wl,--fatal-warnings \
    -Wl,-T${LINKER_SCRIPT} \
    -Wl,--start-group -lstdc++ -lc -lm -Wl,--end-group \
    -Wl,--print-memory-usage \
    -Wl,--save-temps \
")

# Build information autocoder configuration
set(BUILD_INFO_AC_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/../autocoders/generate_build_info.py")
set(BUILD_INFO_AC_DIR "${CMAKE_BINARY_DIR}/Deployments/build_info")
make_directory("${BUILD_INFO_AC_DIR}")

# FIXME - Temporary until F' cmake system creates this folder (only `fprime-util generate` currently creates the folder)
# FIXME - Folder needed for hash-to-file util
# FIXME - Related F' ticket: https://github.com/nasa/fprime/issues/4032
make_directory("${CMAKE_BINARY_DIR}/.fprime-build-dir")

# Call this in the deployment CMakeLists after register_fprime_deployment to
# ensure App.hex is generated in addition to App.elf file
# register_with_bsp("${PROJECT_NAME}")
function(register_with_bsp TARGET_NAME)
    # Differentiate the elf from hex format
    set_target_properties("${TARGET_NAME}" PROPERTIES SUFFIX ".elf")
    # Rebuild if linker script changed
    set_target_properties("${TARGET_NAME}" PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
    # Generate map file
    target_link_options("${TARGET_NAME}" PRIVATE "-Wl,-Map=$<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.map")
    # Generate deployment build information prior to linking
    set(BUILD_INFO_AC_CPP "${BUILD_INFO_AC_DIR}/${TARGET_NAME}_BuildInfoAc.cpp")
    set(BUILD_INFO_AC_OBJ "${BUILD_INFO_AC_CPP}.obj")
    # This is needed to expand the CMAKE_CXX_FLAGS variable into a list for use inside add_custom_command
    set(BUILD_INFO_AC_CMD "${CMAKE_CXX_COMPILER}" -c "${BUILD_INFO_AC_CPP}" -o "${BUILD_INFO_AC_OBJ}")
    separate_arguments(BUILD_INFO_AC_CXX_FLAGS UNIX_COMMAND "${CMAKE_CXX_FLAGS}")
    foreach(AC_CXX_FLAG ${BUILD_INFO_AC_CXX_FLAGS})
        list(APPEND BUILD_INFO_AC_CMD ${AC_CXX_FLAG})
    endforeach()
    # Add (a) all build directories as include paths and (b) special directories from CMAKE_BINARY_DIR
    # The way in which the include paths in CMAKE_BINARY_DIR are added is a little ugly, but the
    # '${CMAKE_BINARY_DIR}/F-Prime/${_FP_PACKAGE_DIR}' paths aren't added to any global variables
    # accessible here, same for the platform headers
    list(APPEND BUILD_INFO_AC_CMD "-I${CMAKE_BINARY_DIR}/F-Prime/default/")
    list(APPEND BUILD_INFO_AC_CMD "-I${CMAKE_BINARY_DIR}/cmake/platform/va416x0/")
    foreach(BUILD_DIR ${FPRIME_BUILD_LOCATIONS})
        list(APPEND BUILD_INFO_AC_CMD "-I${BUILD_DIR}")
    endforeach()
    add_custom_command("TARGET" "${TARGET_NAME}" PRE_LINK
        # Auto-generate the CPP file containing the build information
        COMMAND "${CMAKE_COMMAND}" -E env
            "FPRIME_PROJECT_ROOT=${FPRIME_PROJECT_ROOT}"
            "${PYTHON}" "${BUILD_INFO_AC_SCRIPT}" "${TARGET_NAME}" "${BUILD_INFO_AC_CPP}"
        # Then compile to an object file to be linked to the target
        COMMAND ${BUILD_INFO_AC_CMD}
    )
    # Add the auto-generated object file to the linker path
    target_link_options("${TARGET_NAME}" PRIVATE "${BUILD_INFO_AC_OBJ}")
    # Do a few post-build steps that are not built in
    add_custom_command("TARGET" "${TARGET_NAME}" POST_BUILD
        # Copy the map file into the build-artifacts directory
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.map"
            "${CMAKE_INSTALL_PREFIX}/${TOOLCHAIN_NAME}/${TARGET_NAME}/bin/"
        # Create the hex format for flash loader
        COMMAND arm-none-eabi-objcopy -O ihex
            "$<TARGET_FILE:${TARGET_NAME}>"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.hex"
            DEPENDS "$<TARGET_FILE:${TARGET_NAME}>"
        # Copy the new hex file into the build-artifacts directory
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.hex"
            "${CMAKE_INSTALL_PREFIX}/${TOOLCHAIN_NAME}/${TARGET_NAME}/bin/"
        # Create the bin format for flash loader
        COMMAND arm-none-eabi-objcopy -O binary
            "$<TARGET_FILE:${TARGET_NAME}>"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.bin"
            DEPENDS "$<TARGET_FILE:${TARGET_NAME}>"
        # Copy the new bin file into the build-artifacts directory
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.bin"
            "${CMAKE_INSTALL_PREFIX}/${TOOLCHAIN_NAME}/${TARGET_NAME}/bin/"
        # Objdump the ELF file
        COMMAND arm-none-eabi-objdump -xD --visualize-jumps "$<TARGET_FILE:${TARGET_NAME}>"
            >"$<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.objdump"
            DEPENDS "$<TARGET_FILE:${TARGET_NAME}>"
        # Copy the dump into the build-artifacts directory
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.objdump"
            "${CMAKE_INSTALL_PREFIX}/${TOOLCHAIN_NAME}/${TARGET_NAME}/bin/"
    )
endfunction()
