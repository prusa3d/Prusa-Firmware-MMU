# define the test executable
add_executable(leds_tests ../../../../src/modules/leds.cpp test_leds.cpp)

# define required search paths
target_include_directories(
  leds_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
  )

# tell build system about the test case
add_catch_test(leds_tests)
