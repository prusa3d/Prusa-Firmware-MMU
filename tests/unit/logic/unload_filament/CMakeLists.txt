# define the test executable
add_executable(
  unload_filament_tests
  ../../../../src/logic/feed_to_finda.cpp
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
  test_unload_filament.cpp
  )

# define required search paths
target_include_directories(
  unload_filament_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                               ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(unload_filament_tests)
