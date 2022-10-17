# first, let's create a symbolic target for all tests
add_custom_target(tests)

add_compile_definitions(UNITTEST)

# include doctest_discover_tests function from doctest
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

add_library(doctest_main ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp)
target_link_libraries(doctest_main doctest::doctest)

# registers given target as a catch test
function(add_doctest_test target)
  target_link_libraries(${target} doctest_main doctest::doctest)
  doctest_discover_tests(${target})
  add_dependencies(tests ${target})
endfunction()

add_executable(system_tests ${CMAKE_CURRENT_SOURCE_DIR}/system_test.cpp)
add_doctest_test(system_tests)

set(MODULES_STUBS_DIR ${CMAKE_SOURCE_DIR}/tests/unit/modules/stubs)
set(LOGIC_STUBS_DIR ${CMAKE_SOURCE_DIR}/tests/unit/logic/stubs)

# now, include all the unit tests; they should add themselves using the add_doctest_test function
add_subdirectory(hal)
add_subdirectory(logic)
add_subdirectory(modules)

add_subdirectory(application)
