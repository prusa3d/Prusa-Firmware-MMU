# define the test executable
add_executable(
  motion_tests
  ${CMAKE_SOURCE_DIR}/src/modules/motion.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/speed_table.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/pulse_gen.cpp
  ${MODULES_STUBS_DIR}/stub_gpio.cpp
  ${MODULES_STUBS_DIR}/stub_shr16.cpp
  ${MODULES_STUBS_DIR}/stub_tmc2130.cpp
  test_motion.cpp
  )

# define required search paths
target_include_directories(
  motion_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
  )

# tell build system about the test case
add_catch_test(motion_tests)
