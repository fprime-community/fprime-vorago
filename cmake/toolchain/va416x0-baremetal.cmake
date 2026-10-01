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
#   https://github.com/fprime-community/llvm-vorago-arm-toolchain
# See README.md for suggested development container.
set(CMAKE_C_COMPILER clang-23)
set(CMAKE_CXX_COMPILER clang-23)
set(CMAKE_ASM_COMPILER clang-23)

# FIXME: This is only needed because our linker script and linker arguments
#  necessitate the use of symbols provided by Va416x0/Svc/VectorTable.
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_ASM_COMPILER_WORKS 1)

set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/va416x0.ld)
set(SCRIPT_VERIFY_NO_STRB "${CMAKE_CURRENT_LIST_DIR}/verify_nostrb.py")
set(SCRIPT_COMBINE_IMAGE "${CMAKE_CURRENT_LIST_DIR}/../scripts/stamp_image.py")

# Must match the size of CODE_SRAM in the LINKER_SCRIPT
set(VA416X0_CODE_SRAM_SIZE 0x40000) # 256KiB

# VA416X0_APP_BASE splits CODE_SRAM between a stub bootloader [0, VA416X0_APP_BASE) and the deployment 
# it boots [VA416X0_APP_BASE, VA416X0_CODE_SRAM_END), so that each can be linked and installed separately.
if (NOT DEFINED VA416X0_APP_BASE)
    set(VA416X0_APP_BASE 0x2000) # 8KiB (1/32 of 256KiB)
endif()

# Bytes reserved at the end of CODE_SRAM (e.g. for a board ID) that no image will write to.
# Images end at VA416X0_CODE_SRAM_END instead, so an image check is stamped just below the reserved bytes.
if (NOT DEFINED VA416X0_CODE_SRAM_RESERVED_SIZE)
    set(VA416X0_CODE_SRAM_RESERVED_SIZE 0)
endif()
math(EXPR VA416X0_CODE_SRAM_END "${VA416X0_CODE_SRAM_SIZE} - ${VA416X0_CODE_SRAM_RESERVED_SIZE}" OUTPUT_FORMAT HEXADECIMAL)

# Define `VA416X0_MCPU` to override the `-mcpu` compiler flag to enable
# additional compiler features.
if (NOT DEFINED VA416X0_MCPU)
    set(VA416X0_MCPU "cortex-m4")
endif()

set(VA416X0_COMMON_FLAGS "\
    --target=thumbv7m-unknown-none-eabi \
    -mcpu=${VA416X0_MCPU} \
    -mthumb \
    -mno-unaligned-access \
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

# Verify that the libc selected was built without unaligned-access enabled
include("${CMAKE_CURRENT_LIST_DIR}/check_library_unaligned.cmake")


# Call this in the deployment CMakeLists after register_fprime_deployment to
# ensure App.hex is generated in addition to App.elf file
# register_with_bsp("${PROJECT_NAME}" [IS_BOOTLOADER] [BOOTLOADER <bootloader-target>...] [IMAGE_CHECK <crc32>])
#   @param IS_BOOTLOADER: link as a stub bootloader into [0, VA416X0_APP_BASE) of CODE_SRAM
#   @param BOOTLOADER:    link as a deployment booted by a stub bootloader into [VA416X0_APP_BASE, 256K) of CODE_SRAM
#                         generates a combined <target>_with_<bootloader>.bin for each bootloader target given
#   @param IMAGE_CHECK:   generates a <target>_${IMAGE_CHECK}_stamped.img, stamped with the given image check
function(register_with_bsp TARGET_NAME)
    cmake_parse_arguments(PARSE_ARGV 1 BSP "IS_BOOTLOADER" "IMAGE_CHECK" "BOOTLOADER")
    if (BSP_IS_BOOTLOADER AND (BSP_BOOTLOADER OR DEFINED BSP_IMAGE_CHECK))
        message(FATAL_ERROR "register_with_bsp(${TARGET_NAME}): IS_BOOTLOADER cannot be combined with BOOTLOADER or IMAGE_CHECK")
    endif()
    # Bytes reserved at the end of the image for the image check
    set(IMAGE_CHECK_SIZE 0)
    if (DEFINED BSP_IMAGE_CHECK AND BSP_IMAGE_CHECK STREQUAL "crc32")
        set(IMAGE_CHECK_SIZE 4)
    elseif(DEFINED BSP_IMAGE_CHECK)
        message(FATAL_ERROR "register_with_bsp(${TARGET_NAME}): unknown IMAGE_CHECK '${BSP_IMAGE_CHECK}'")
    endif()
    # Select the part of CODE_SRAM this image is linked into, less the space for the image check
    set(IMAGE_START 0)
    set(IMAGE_END "${VA416X0_CODE_SRAM_END}")
    if (BSP_IS_BOOTLOADER)
        set(IMAGE_END "${VA416X0_APP_BASE}")
    elseif (BSP_BOOTLOADER)
        set(IMAGE_START "${VA416X0_APP_BASE}")
    endif()
    math(EXPR IMAGE_LINK_END "${IMAGE_END} - ${IMAGE_CHECK_SIZE}" OUTPUT_FORMAT HEXADECIMAL)

    target_link_options("${TARGET_NAME}" PRIVATE
        "-Wl,--defsym=__image_start=${IMAGE_START}"
        "-Wl,--defsym=__image_end=${IMAGE_LINK_END}"
        "-Wl,--defsym=__app_image_start=${VA416X0_APP_BASE}"
        "-Wl,--defsym=__app_image_end=${VA416X0_CODE_SRAM_END}"
    )

    set(OUT_BASE "$<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>")
    set(OUT_BIN "${FPRIME_INSTALL_DEST}/${TOOLCHAIN_NAME}/${TARGET_NAME}/bin/")
    # Differentiate the elf from hex format
    set_target_properties("${TARGET_NAME}" PROPERTIES SUFFIX ".elf")
    # Rebuild if linker script changed
    set_target_properties("${TARGET_NAME}" PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
    # Generate map file
    target_link_options("${TARGET_NAME}" PRIVATE "-Wl,-Map=${OUT_BASE}.map")
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
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${OUT_BASE}.map" "${OUT_BIN}"
        # Create the hex format for flash loader
        COMMAND arm-none-eabi-objcopy -O ihex
            "$<TARGET_FILE:${TARGET_NAME}>"
            "${OUT_BASE}.hex"
            DEPENDS "$<TARGET_FILE:${TARGET_NAME}>"
        # Copy the new hex file into the build-artifacts directory
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${OUT_BASE}.hex" "${OUT_BIN}"
        # Create the bin format for flash loader
        COMMAND arm-none-eabi-objcopy -O binary
            "$<TARGET_FILE:${TARGET_NAME}>"
            "${OUT_BASE}.bin"
            DEPENDS "$<TARGET_FILE:${TARGET_NAME}>"
        # Copy the new bin file into the build-artifacts directory
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${OUT_BASE}.bin" "${OUT_BIN}"
        # Objdump the ELF file
        COMMAND arm-none-eabi-objdump -xD --visualize-jumps "$<TARGET_FILE:${TARGET_NAME}>"
            >"${OUT_BASE}.objdump"
            DEPENDS "$<TARGET_FILE:${TARGET_NAME}>"
        # Copy the dump into the build-artifacts directory
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${OUT_BASE}.objdump" "${OUT_BIN}"
    )

    if (VA416X0_VERIFY_NO_STRB)
        # Define the default set of STRB whitelist
        set(VA416X0_VERIFY_NO_STRB_WHITELIST
            ${VA416X0_VERIFY_NO_STRB_WHITELIST}

            # We actually _do_ want 'strb' in 'Va416x0Mmio::Amba::write_u8'
            # This uses inline assembly to avoid the `badstrb` feature
            _ZN11Va416x0Mmio4Amba8write_u8Ejh

            # V-table data stored in .text
            __start___lcxx_override

            # C vector table for initializing arrays
            __preinit_array_start
            __bothinit_array_start
            __postinit_array_start
            __init_array_start

            # Other linker labels
            __text_end
        )

        add_custom_command("TARGET" "${TARGET_NAME}" POST_BUILD
            # Verify the .text of the objdump does not include illegal instructions
            COMMAND "${PYTHON}" "${SCRIPT_VERIFY_NO_STRB}"
                    "$<TARGET_FILE:${TARGET_NAME}>"
                    --whitelist "\"${VA416X0_VERIFY_NO_STRB_WHITELIST}\""
                    --name ${TARGET_NAME}

                DEPENDS "$<TARGET_FILE:${TARGET_NAME}>"
        )
    endif()

    set(STAMP_IMAGE_CMD "${PYTHON}" "${SCRIPT_COMBINE_IMAGE}" --image-start "${IMAGE_START}" --image-end "${IMAGE_END}")
    if (DEFINED BSP_IMAGE_CHECK)
        list(APPEND STAMP_IMAGE_CMD --image-check "${BSP_IMAGE_CHECK}")
        add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
            # Stamp the image check into the image installed at the start of its CODE_SRAM region
            COMMAND ${STAMP_IMAGE_CMD} "${OUT_BASE}.bin" "${OUT_BASE}_${BSP_IMAGE_CHECK}_stamped.img"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${OUT_BASE}_${BSP_IMAGE_CHECK}_stamped.img" "${OUT_BIN}"
        )
    endif()

    foreach(BOOTLOADER_TARGET IN LISTS BSP_BOOTLOADER)
        # Relink when the bootloader changes, so the combined image is regenerated with it
        add_dependencies("${TARGET_NAME}" "${BOOTLOADER_TARGET}")
        set_property(TARGET "${TARGET_NAME}" APPEND PROPERTY LINK_DEPENDS "$<TARGET_FILE:${BOOTLOADER_TARGET}>")
        add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
            # Combine the bootloader and the stamped image into one image for the whole of CODE_SRAM
            COMMAND ${STAMP_IMAGE_CMD}
                --bootloader "$<TARGET_FILE_DIR:${BOOTLOADER_TARGET}>/$<TARGET_FILE_BASE_NAME:${BOOTLOADER_TARGET}>.bin"
                "${OUT_BASE}.bin" "${OUT_BASE}_with_${BOOTLOADER_TARGET}.bin"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${OUT_BASE}_with_${BOOTLOADER_TARGET}.bin" "${OUT_BIN}"
        )
    endforeach()
endfunction()
