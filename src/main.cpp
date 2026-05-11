#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <filesystem>

#include "ast.hpp"
#include "dot_print.hpp"
#include "semantic.hpp"
#include "flags.hpp"

extern FILE* yyin;
extern int yyparse();
extern std::unique_ptr<BlockNode> rootBlock;

namespace fs = std::filesystem;


int main(int argc, char** argv) {

    const CliOptions opts = parse_args(argc, argv);

    FILE* file = fopen(opts.input_file.c_str(), "r");
    if (!file) {
        std::cerr << "senna: cannot open '" << opts.input_file << "'\n";
        return 1;
    }

    yyin = file;

    rootBlock = std::make_unique<BlockNode>();

    if (yyparse() == 0) {
        std::cout << "[+] Parsing successful!" << std::endl;

        SemanticAnalyzer semantic_checker;
        rootBlock->accept(semantic_checker);

        if (semantic_checker.has_errors()) {
            std::cerr << "[-] Semantic compilation failed due to errors above." << std::endl;
            fclose(file);
            return 1;
        }
        std::cout << "[+] Semantic analysis passed!" << std::endl;

        if (opts.has_emit(EmitTarget::AST)) {

            std::string file_path = "output/dot/ast_output.dot";
            fs::path dir_path = fs::path(file_path).parent_path();

            if (!dir_path.empty() && !fs::exists(dir_path)) {
                fs::create_directories(dir_path);
            }

            std::ofstream out_file(file_path);
            if (!out_file.is_open()) {
                std::cerr << "Error: Could not create output dot file." << std::endl;
                fclose(file);
                return 1;
            }

            GraphDump dumper(out_file);

            dumper.header_write();
            rootBlock->accept(dumper);
            dumper.footer_write();

            std::cout << "[+] AST saved to './output/dot/ast_output.dot'" << std::endl;
        }
    } else {
        std::cerr << "[-] Parsing failed.\n";
        fclose(file);
        return 1;
    }

    if (opts.has_emit(EmitTarget::IR)) {
        std::cout << "[+] IR emit: not yet implemented, but optget seems to work for it\n";
    }

    fclose(file);
    return 0;
}
