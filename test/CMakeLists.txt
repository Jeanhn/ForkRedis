
find_package(GTest REQUIRED)

file(GLOB UNIT_TEST_SRC ${PROJECT_SOURCE_DIR}/test/*.cc)

foreach(unit_test_src ${UNIT_TEST_SRC})

    get_filename_component(unit_test_src_filename ${unit_test_src} NAME)
    string(REPLACE ".cc" "" unit_test_exec_filename ${unit_test_src_filename})

    add_executable(${unit_test_exec_filename} ${unit_test_src_filename})

    target_link_libraries(${unit_test_exec_filename} redis)

    target_link_libraries(
    ${unit_test_exec_filename}
        GTest::gtest_main
    )

    target_compile_options(${unit_test_exec_filename} PRIVATE -g)
endforeach()
