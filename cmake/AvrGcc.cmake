get_filename_component(PROJECT_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)

set(AVR_GCC_VERSION 7.3.0)
set(AVR_TOOLCHAIN_DIR "${PROJECT_CMAKE_DIR}/../.dependencies/avr-gcc-${AVR_GCC_VERSION}/")

include("${PROJECT_CMAKE_DIR}/AnyAvrGcc.cmake")
