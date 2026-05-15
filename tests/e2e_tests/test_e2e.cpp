#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

namespace fs = std::filesystem;

std::vector<std::string> read_lines(const fs::path& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

void run_e2e_test(const std::string& test_name) {
    fs::path source_path = fs::path(SENNA_CASES_DIR) / (test_name + ".sn");
    fs::path expected_path = fs::path(SENNA_EXPECTED_DIR) / (test_name + ".txt");

    ASSERT_TRUE(fs::exists(source_path)) << "Source file not found: " << source_path;
    ASSERT_TRUE(fs::exists(expected_path)) << "Expected output file not found: " << expected_path;

    std::string compile_cmd = std::string(SENNA_COMPILER_PATH) + " " +
                              source_path.string() + " > temp_compile.log 2>&1";
    int compile_res = std::system(compile_cmd.c_str());

    if (compile_res != 0) {
        std::ifstream log("temp_compile.log");
        std::string log_content((std::istreambuf_iterator<char>(log)), std::istreambuf_iterator<char>());
        std::remove("temp_compile.log");
        FAIL() << "Compilation failed for " << test_name << ".sn\nLog:\n" << log_content;
    }
    std::remove("temp_compile.log");

    std::string bin_path = "./output/" + test_name;
    ASSERT_TRUE(fs::exists(bin_path)) << "Binary not found after compilation: " << bin_path;

    std::string run_cmd = bin_path + " > temp_run.log 2>&1";
    int run_res = std::system(run_cmd.c_str());

    ASSERT_EQ(run_res, 0) << "Runtime crash in " << bin_path;

    std::vector<std::string> actual_output = read_lines("temp_run.log");
    std::vector<std::string> expected_output = read_lines(expected_path);

    std::remove("temp_run.log");
    std::remove(bin_path.c_str());

    ASSERT_EQ(actual_output.size(), expected_output.size())
        << "Mismatch in number of output lines for test: " << test_name;

    for (size_t i = 0; i < expected_output.size(); ++i) {
        EXPECT_EQ(actual_output[i], expected_output[i])
            << "Mismatch at line " << (i + 1) << " in test " << test_name;
    }
}

TEST(SennaE2ETest, EdgeEmpty)
{
    run_e2e_test("01_edge_empty");
}

TEST(SennaE2ETest, MathPrecedence)
{
    run_e2e_test("02_math_precedence");
}

TEST(SennaE2ETest, DivByZeroFold)
{
    run_e2e_test("03_div_by_zero_fold");
}

TEST(SennaE2ETest, FibRecursion)
{
    run_e2e_test("04_fib_recursion");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
