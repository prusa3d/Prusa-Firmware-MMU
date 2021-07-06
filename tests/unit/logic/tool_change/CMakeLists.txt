# define the test executable
add_executable(
  tool_change_tests
  ../../../../src/logic/feed_to_bondtech.cpp
  ../../../../src/logic/feed_to_finda.cpp
  ../../../../src/logic/load_filament.cpp
  ../../../../src/logic/tool_change.cpp
  ../../../../src/logic/unload_filament.cpp
  ../../../../src/logic/unload_to_finda.cpp
  ../../../../src/modules/buttons.cpp
  ../../../../src/modules/debouncer.cpp
  ../../../../src/modules/finda.cpp
  ../../../../src/modules/fsensor.cpp
  ../../../../src/modules/globals.cpp
  ../../../../src/modules/idler.cpp
  ../../../../src/modules/leds.cpp
  ../../../../src/modules/permanent_storage.cpp
  ../../../../src/modules/selector.cpp
  ../../../../src/modules/user_input.cpp
  ../../../../src/modules/pulse_gen.cpp
  ../../modules/stubs/stub_adc.cpp
  ../../modules/stubs/stub_eeprom.cpp
  ../../modules/stubs/stub_gpio.cpp
  ../../modules/stubs/stub_shr16.cpp
  ../../modules/stubs/stub_timebase.cpp
  ../../modules/stubs/stub_tmc2130.cpp
  ../stubs/main_loop_stub.cpp
  ../stubs/stub_motion.cpp
  test_tool_change.cpp
  )

# define required search paths
target_include_directories(
  tool_change_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                           ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(tool_change_tests)
