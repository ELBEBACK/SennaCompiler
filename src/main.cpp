#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <filesystem>

#include "ast.hpp"
#include "dot_print.hpp"
#include "semantic.hpp"
#include "flags.hpp"
#include "ir.hpp"
#include "ir_builder.hpp"
#include "ir_print.hpp"
#include "cfg_builder.hpp"
#include "cfg_print.hpp"
#include "ir_verify.hpp"


extern FILE* yyin;
extern int yyparse(std::vector<std::string>& syntax_errors);
extern std::unique_ptr<BlockNode> rootBlock;

namespace fs = std::filesystem;


static bool ensure_dir(const std::string& file_path) {
    fs::path dir = fs::path(file_path).parent_path();
    if (!dir.empty() && !fs::exists(dir)) {
        std::error_code ec;
        fs::create_directories(dir, ec);
        if (ec) {
            std::cerr << "senna: cannot create '"
                      << dir.string() << "': " << ec.message() << "\n";
            return false;
        }
    }
    return true;
}


int main(int argc, char** argv) {

    const CliOptions opts = parse_args(argc, argv);

    std::vector<std::string> syntax_errors;

    FILE* file = fopen(opts.input_file.c_str(), "r");
    if (!file) {
        std::cerr << "senna: cannot open '" << opts.input_file << "'\n";
        return 1;
    }

    yyin = file;

    rootBlock = std::make_unique<BlockNode>();

    if (yyparse(syntax_errors) == 0) {
        if (!syntax_errors.empty()) {
            for (const auto& err : syntax_errors) std::cerr << err << std::endl;
        }
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
            ensure_dir(file_path);

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

            std::cout << "[+] AST has been saved to './output/dot/ast_output.dot'" << std::endl;
        }
    } else {
        std::cerr << "[-] Parsing failed.\n";
        fclose(file);
        return 1;
    }


    IRBuilder builder;
    Module    mod = builder.build(*rootBlock);

    IRValidator validator;
    if (!validator.validate(mod)) {
        std::cerr << "[-] IR Verification failed!" << std::endl;
        for (const auto& err : validator.get_errors()) {
            std::cerr << "  - " << err.function_name << " [" << err.block_label << "]: " << err.message << std::endl;
        }
        return 1;
    }
    std::cout << "[+] IR Verification passed!" << std::endl;

    if (opts.has_emit(EmitTarget::IR)) {

        if (opts.has_emit(EmitTarget::IR)) {
            const std::string path = "output/out.ir";
            if (!ensure_dir(path)) { fclose(file); return 1; }
            std::ofstream out(path);
            IRPrinter printer(out);
            printer.print(mod);
            std::cout << "[+] Non-SSA IR has been saved to ./output/out.ir" << "\n";
        }
    }

    CFGBuilder cfg_builder;
    cfg_builder.build(mod);

    if (opts.has_emit(EmitTarget::CFG)) {
        const std::string dot_dir = "output/dot";
        if (!ensure_dir(dot_dir + "/dummy.dot")) {
            fclose(file);
            return 1;
        }

        for (auto& fn : mod.functions) {
            std::string path = dot_dir + "/cfg_" + fn->name + ".dot";
            std::ofstream out(path);
            if (out.is_open()) {
                CFGPrinter printer(out);
                printer.print_fn(*fn);
                std::cout << "[+] CFG for function @" << fn->name << " saved to " << path << "\n";
            }
        }
    }

    if (opts.has_emit(EmitTarget::DOM)) {
        std::cout << "[+] domtree emit: not yet implemented, but optget seems to work for it\n";
    }

    if (opts.has_emit(EmitTarget::FDOM)) {
        std::cout << "[+] dom frontiers emit: not yet implemented, but optget seems to work for it\n";
    }

    if (opts.has_emit(EmitTarget::SSA)) {
        std::cout << "[+] SSA IR emit: not yet implemented, but optget seems to work for it\n";
    }

    if (opts.has_emit(EmitTarget::LOOPS)) {
        std::cout << "[+] looptree emit: not yet implemented, but optget seems to work for it\n";
    }

    if (opts.has_emit(EmitTarget::LLVM)) {
        std::cout << "[+] .ll emit: not yet implemented, but optget seems to work for it\n";
    }

    fclose(file);
    return 0;
}
