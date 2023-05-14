
find_package(GTest REQUIRED)

file(GLOB UNIT_TEST_SRC ${PROJECT_SOURCE_DIR}/test/*.cc)

foreach(unit_test_src ${UNIT_TEST_SRC})

    get_filename_component(unit_test_src_filename ${unit_test_src} NAME)
    string(REPLACE ".cc" "" unit_test_exec_filename ${unit_test_src_filename})

    add_executable(${unit_test_exec_filename} ${unit_test_src_filename})

    target_link_libraries(${unit_test_exec_filename} rds::lib lzf::lib)

    target_link_libraries(
    ${unit_test_exec_filename}
        GTest::gtest_main
    )
endforeach()


target_link_libraries(test_str rds::lib)

target_link_libraries(
  test_str
  GTest::gtest_main
)