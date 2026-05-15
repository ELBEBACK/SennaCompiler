add_executable(senna_e2e_tests test_e2e.cpp)

target_link_libraries(senna_e2e_tests PRIVATE GTest::gtest_main)

target_compile_definitions(senna_e2e_tests PRIVATE
    SENNA_COMPILER_PATH="${PROJECT_BINARY_DIR}/src/senna"
    SENNA_CASES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/cases"
    SENNA_EXPECTED_DIR="${CMAKE_CURRENT_SOURCE_DIR}/expected"
)

include(GoogleTest)
gtest_discover_tests(senna_e2e_tests)
