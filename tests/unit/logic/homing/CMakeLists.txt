# define the test executable
add_executable(homing_tests test_homing.cpp)

# define required search paths
target_include_directories(
  homing_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                      ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(homing_tests)
