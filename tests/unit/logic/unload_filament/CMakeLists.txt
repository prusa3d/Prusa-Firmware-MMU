# define the test executable
add_executable(unload_filament_tests test_unload_filament.cpp)

# define required search paths
target_include_directories(
  unload_filament_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                               ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(unload_filament_tests)
