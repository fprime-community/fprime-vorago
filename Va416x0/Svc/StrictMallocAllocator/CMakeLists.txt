####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
if(FPRIME_BAREMENTAL_OVERRIDE_NEW_DELETE)

register_fprime_module(
    Va416x0_Svc_StrictMallocAllocator
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/StrictMallocAllocator.cpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/StrictMallocAllocator.hpp"
    DEPENDS
        Fw_Types
        Os_Baremetal_OverrideNewDelete
)
endif()
