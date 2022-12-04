# define the test executable
add_executable(eject_filament_tests test_eject_filament.cpp)

# define required search paths
target_include_directories(
  eject_filament_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                              ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(eject_filament_tests)
