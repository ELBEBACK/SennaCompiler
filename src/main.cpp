#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include "ast.hpp"
#include "dot_print.hpp"

extern FILE* yyin;
extern int yyparse();
extern std::unique_ptr<BlockNode> rootBlock;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.pcl> [--emit=ast]" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    bool emit_ast = false;

    if (argc > 2 && std::string(argv[2]) == "--emit=ast") {
        emit_ast = true;
    }

    FILE* file = fopen(input_file.c_str(), "r");
    if (!file) {
        std::cerr << "Error: Could not open file " << input_file << std::endl;
        return 1;
    }

    yyin = file;

    rootBlock = std::make_unique<BlockNode>();

    if (yyparse() == 0) {
        std::cout << "[+] Parsing successful!" << std::endl;

        if (emit_ast) {
            std::ofstream out_file("ast_output.dot");
            if (!out_file.is_open()) {
                std::cerr << "Error: Could not create output dot file." << std::endl;
                fclose(file);
                return 1;
            }

            GraphDump dumper(out_file);

            dumper.header_write();
            rootBlock->accept(dumper);
            dumper.footer_write();

            std::cout << "[+] AST saved to 'ast_output.dot'" << std::endl;
        }
    } else {
        std::cerr << "[-] Parsing failed due to syntax errors." << std::endl;
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}
