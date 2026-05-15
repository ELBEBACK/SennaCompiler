add_executable(senna_unit_tests
    test_ast.cpp
    test_semantic.cpp
    test_ir.cpp
    test_dom.cpp
    test_opts.cpp
)

target_link_libraries(senna_unit_tests PRIVATE senna_compiler GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(senna_unit_tests)
