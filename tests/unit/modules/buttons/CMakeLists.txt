# define the test executable
add_executable(
  buttons_tests ../../../../src/modules/buttons.cpp ../../../../src/modules/debouncer.cpp
                ../stubs/stub_adc.cpp ../stubs/stub_timebase.cpp test_buttons.cpp
  )

# define required search paths
target_include_directories(
  buttons_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
  )

# tell build system about the test case
add_catch_test(buttons_tests)
