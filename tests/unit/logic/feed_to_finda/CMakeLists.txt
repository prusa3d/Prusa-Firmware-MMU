# define the test executable
add_executable(feed_to_finda_tests test_feed_to_finda.cpp)

# define required search paths
target_include_directories(
  feed_to_finda_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_SOURCE_DIR}/src/hal
                             ${CMAKE_SOURCE_DIR}/src/logic
  )

# tell build system about the test case
add_catch_test(feed_to_finda_tests)
