# define the test executable
add_executable(application_tests test_application.cpp test_registers.cpp)

# define required search paths
target_include_directories(
  application_tests PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/modules
                           ${CMAKE_SOURCE_DIR}/src/hal
  )

# tell build system about the test case
add_catch_test(application_tests)
