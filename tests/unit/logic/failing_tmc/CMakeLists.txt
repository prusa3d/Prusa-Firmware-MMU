# define the test executable
add_executable(failing_tmc_tests test_failing_tmc.cpp)

# define required search paths
target_include_directories(
  failing_tmc_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                           ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(failing_tmc_tests)
