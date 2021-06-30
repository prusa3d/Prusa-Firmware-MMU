option(GCOV_ENABLE "Enable/use GCOV code coverage for tests" ON)
option(GCOV_CONTINUE_ON_FAILURE "Continue and generate test report even if tests fail" ON)

if (GCOV_ENABLE)
	find_program(GCOV_BINARY NAMES gcov gcov-7)
    find_program(LCOV_BINARY NAMES lcov)
	message(STATUS "Using gcov ${GCOV_BINARY} and LCOV ${LCOV_BINARY}")
	add_compile_options(-g -O0 -fprofile-arcs -ftest-coverage)
	link_libraries(-coverage -lgcov)
	set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
    add_custom_target(tests_clean
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/Coverage
        COMMAND lcov -b ${PROJECT_BINARY_DIR} -d ${PROJECT_BINARY_DIR} --zerocounters
    )

    # TODO - right now this is kinda janky because some of the tests fail and this causes execution to abort. 
    # Once they no longer do that we can merge this into the test_coverage_report step so it all runs seamlessly. 
    # For now, build test_run_all and ignore the failure. Then build the test_coverage_report target.
    add_custom_target(test_run_all
        COMMAND cd ${PROJECT_BINARY_DIR} && env CTEST_OUTPUT_ON_FAILURE=1 ctest --timeout 30 || exit 0
        DEPENDS tests_clean tests
    )

    add_custom_target(test_coverage_report
        COMMAND cd ${PROJECT_BINARY_DIR}
        COMMAND ${LCOV_BINARY} --capture --gcov-tool="${GCOV_BINARY}" --directory ${PROJECT_BINARY_DIR} --output-file ${PROJECT_BINARY_DIR}/coverage.info
        # Strip system headers and other uninteresting stuff. 
        COMMAND ${LCOV_BINARY} --remove coverage.info '/usr/*' '${PROJECT_SOURCE_DIR}/tests/*' '${PROJECT_SOURCE_DIR}/lib/Catch2/*' -o coverage.info
        # Package it up. 
        COMMAND genhtml coverage.info --output-directory Coverage
        COMMAND tar -zcvf Coverage.tar.gz Coverage
        # TODO - this needs to be fixed so it doesn't suck in all the catch/test cruft.
    	#COMMAND ${PROJECT_SOURCE_DIR}/utils/gcovr.py -r ${PROJECT_SOURCE_DIR} -e '${PROJECT_SOURCE_DIR}/tests/*' -e Catch2 | tee ${PROJECT_BINARY_DIR}/summary.txt
        #DEPENDS tests_clean tests
    )
else()
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE OFF)
endif()

add_subdirectory(unit)
