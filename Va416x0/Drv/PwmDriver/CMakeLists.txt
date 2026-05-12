####
# F Prime CMakeLists.txt:
#
# SOURCES: list of source files (to be compiled)
# AUTOCODER_INPUTS: list of files to be passed to the autocoders
# DEPENDS: list of libraries that this module depends on
#
# More information in the F´ CMake API documentation:
# https://fprime.jpl.nasa.gov/latest/docs/reference/api/cmake/API/
#
####

register_fprime_library(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/PwmDriver.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/PwmDriver.cpp"
  DEPENDS
        Va416x0_Mmio_Timer
)

### Unit Tests ###
# register_fprime_ut(
#     AUTOCODER_INPUTS
#         "${CMAKE_CURRENT_LIST_DIR}/PwmDriver.fpp"
#     SOURCES
#         "${CMAKE_CURRENT_LIST_DIR}/test/ut/PwmDriverTestMain.cpp"
#         "${CMAKE_CURRENT_LIST_DIR}/test/ut/PwmDriverTester.cpp"
#     DEPENDS
#         STest # For rules-based testing
#     UT_AUTO_HELPERS
# )
