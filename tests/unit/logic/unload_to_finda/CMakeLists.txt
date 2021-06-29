# define the test executable
add_executable(
  unload_to_finda_tests
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
  ../../modules/stubs/stub_adc.cpp
  ../../modules/stubs/stub_eeprom.cpp
  ../../modules/stubs/stub_shr16.cpp
  ../../modules/stubs/stub_timebase.cpp
  ../stubs/main_loop_stub.cpp
  ../stubs/stub_motion.cpp
  test_unload_to_finda.cpp
  )

# define required search paths
target_include_directories(
  unload_to_finda_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                               ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(unload_to_finda_tests)
