# define the test executable
add_executable(load_filament_tests test_load_filament.cpp)

# define required search paths
target_include_directories(
  load_filament_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                             ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(load_filament_tests)
