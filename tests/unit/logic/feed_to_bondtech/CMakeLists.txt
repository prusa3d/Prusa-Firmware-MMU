# define the test executable
add_executable(feed_to_bondtech_tests test_feed_to_bondtech.cpp)

# define required search paths
target_include_directories(
  feed_to_bondtech_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                                ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(feed_to_bondtech_tests)
