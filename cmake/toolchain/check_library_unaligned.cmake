# Check that the selected C library was not built with unaligned access support
# This is critical because CCR_UNALIGN_TRP is set in fprime-vorago/Va416x0/Svc/VectorTable/

# Build compiler flags string
separate_arguments(C_FLAGS_LIST UNIX_COMMAND "${CMAKE_C_FLAGS}")

# Find the C library that will be linked
execute_process(
    COMMAND ${CMAKE_C_COMPILER} ${C_FLAGS_LIST} -print-file-name=libc.a
    OUTPUT_VARIABLE LIBC_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if (EXISTS "${LIBC_PATH}")
    message(STATUS "Checking library unaligned access support: ${LIBC_PATH}")

    # Use readelf to check the ARM attributes
    execute_process(
        COMMAND arm-none-eabi-readelf -A "${LIBC_PATH}"
        OUTPUT_VARIABLE READELF_OUTPUT
        ERROR_QUIET
    )

    # Check if Tag_CPU_unaligned_access is present
    string(REGEX MATCH "Tag_CPU_unaligned_access: ([^\n]+)" UNALIGNED_MATCH "${READELF_OUTPUT}")

    if (UNALIGNED_MATCH)
        string(SUBSTRING "${READELF_OUTPUT}" 0 1000 READELF_OUTPUT_TRUNC)
        message(INFO "${READELF_OUTPUT_TRUNC}")
        set(UNALIGNED_VALUE "${CMAKE_MATCH_1}")
        # FIXME: changed fatal to a log message
        message(FATAL_ERROR 
            "Selected C library was built with unaligned access support: ${UNALIGNED_VALUE}n"
            "Library: ${LIBC_PATH}\n"
            "fprime-vorago does not allow unaligned memory accesses.\n"
            "A library built with -mno-unaligned-access is required.\n")
    else()
        message(STATUS "Library unaligned access check: OK (no unaligned access support detected)")
    endif()
else()
    message(WARNING "Could not find C library to check unaligned access support")
endif()
