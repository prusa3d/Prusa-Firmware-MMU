# define the test executable
add_executable(tool_change_tests test_tool_change.cpp)

# define required search paths
target_include_directories(
  tool_change_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                           ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(tool_change_tests)
