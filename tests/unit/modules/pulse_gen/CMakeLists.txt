# define the test executable
add_executable(pulse_gen_tests
  test_pulse_gen.cpp
  ../../../../src/modules/pulse_gen.cpp
  ../../../../src/modules/speed_table.cpp
  ../stubs/stub_shr16.cpp
  ../stubs/stub_gpio.cpp
)

# define required search paths
target_include_directories(
  pulse_gen_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
)

# tell build system about the test case
add_catch_test(pulse_gen_tests)
