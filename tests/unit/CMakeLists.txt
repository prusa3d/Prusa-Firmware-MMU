# first, let's create a symbolic target for all tests
add_custom_target(tests)

add_compile_definitions(UNITTEST)

# include catch_discover_tests function from Catch2
include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

add_library(catch_main ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp)
target_link_libraries(catch_main Catch2::Catch2WithMain)

set(MODULES_STUBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules/stubs)
set(LOGIC_STUBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/logic/stubs)

add_library(
  application STATIC
  ${CMAKE_SOURCE_DIR}/src/registers.cpp
  ${CMAKE_SOURCE_DIR}/src/application.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/buttons.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/leds.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/globals.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/finda.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/debouncer.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/fsensor.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/idler.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/movable_base.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/permanent_storage.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/protocol.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/pulley.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/selector.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/user_input.cpp
  ${CMAKE_SOURCE_DIR}/src/modules/pulse_gen.cpp
  ${MODULES_STUBS_DIR}/stub_adc.cpp
  ${MODULES_STUBS_DIR}/stub_cpu.cpp
  ${MODULES_STUBS_DIR}/stub_eeprom.cpp
  ${MODULES_STUBS_DIR}/stub_gpio.cpp
  ${MODULES_STUBS_DIR}/stub_shr16.cpp
  ${MODULES_STUBS_DIR}/stub_serial.cpp
  ${MODULES_STUBS_DIR}/stub_timebase.cpp
  ${MODULES_STUBS_DIR}/stub_tmc2130.cpp
  ${LOGIC_STUBS_DIR}/homing.cpp
  ${LOGIC_STUBS_DIR}/main_loop_stub.cpp
  ${LOGIC_STUBS_DIR}/stub_motion.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/no_command.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/command_base.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/cut_filament.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/load_filament.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/move_selector.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/no_command.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/eject_filament.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/tool_change.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/unload_filament.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/home.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/set_mode.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/feed_to_finda.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/feed_to_bondtech.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/retract_from_finda.cpp
  ${CMAKE_SOURCE_DIR}/src/logic/unload_to_finda.cpp
  )

# define required search paths
target_include_directories(
  application PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/modules
                      ${CMAKE_SOURCE_DIR}/src/hal
  )

target_link_libraries(application catch_main Catch2::Catch2WithMain)

# registers given target as a catch test
function(add_catch_test target)
  target_link_libraries(${target} catch_main Catch2::Catch2WithMain application)
  catch_discover_tests(${target})
  add_dependencies(tests ${target})
endfunction()

add_executable(system_tests ${CMAKE_CURRENT_SOURCE_DIR}/system_test.cpp)
add_catch_test(system_tests)

# now, include all the unit tests; they should add themselves using the add_catch_test function
add_subdirectory(hal)
add_subdirectory(logic)
add_subdirectory(modules)
add_subdirectory(application)
