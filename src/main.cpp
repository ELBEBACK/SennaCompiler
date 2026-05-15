#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <filesystem>
#include <cstdlib>

#include "ast.hpp"
#include "ast_print.hpp"
#include "semantic.hpp"
#include "flags.hpp"
#include "ir.hpp"
#include "ir_builder.hpp"
#include "ir_print.hpp"
#include "cfg_builder.hpp"
#include "cfg_print.hpp"
#include "ir_verify.hpp"
#include "dom_tree.hpp"
#include "dom_front.hpp"
#include "doms_print.hpp"
#include "ssa.hpp"
#include "phi_node.hpp"
#include "dce.hpp"
#include "const_fold_prop.hpp"
#include "exp_simplifier.hpp"
#include "llvm_emit.hpp"

extern FILE* yyin;
extern int yyparse(std::vector<std::string>& syntax_errors);
extern std::unique_ptr<BlockNode> rootBlock;

namespace fs = std::filesystem;

static bool ensure_dir(const fs::path& dir) {
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec && !fs::exists(dir)) {
        std::cerr << "senna: cannot create '" << dir.string() << "': " << ec.message() << "\n";
        return false;
    }
    return true;
}


int main(int argc, char** argv) {

    const CliOptions opts = parse_args(argc, argv);

    FILE* file = fopen(opts.input_file.c_str(), "r");
    if (!file) {
        std::cerr << "senna: cannot open '" << opts.input_file << "'\n";
        return 1;
    }

    yyin      = file;
    rootBlock = std::make_unique<BlockNode>();

    std::vector<std::string> syntax_errors;
    if (yyparse(syntax_errors) != 0) {
        std::cerr << "[-] Parsing failed.\n";
        fclose(file);
        return 1;
    }
    for (const auto& err : syntax_errors) std::cerr << err << "\n";
    std::cout << "[+] Parsing successful!\n";

    SemanticAnalyzer semantic;
    rootBlock->accept(semantic);
    if (semantic.has_errors()) {
        std::cerr << "[-] Semantic analysis failed.\n";
        fclose(file);
        return 1;
    }
    std::cout << "[+] Semantic analysis passed!\n";

    if (opts.has_emit(EmitTarget::AST)) {
        const fs::path path = "output/dot/ast_output.dot";
        if (!ensure_dir(path.parent_path())) { fclose(file); return 1; }
        std::ofstream out(path);
        ASTPrint dumper(out);
        dumper.header_write();
        rootBlock->accept(dumper);
        dumper.footer_write();
        std::cout << "[+] AST saved to " << path << "\n";
    }

    IRBuilder builder;
    Module    mod = builder.build(*rootBlock);

    IRValidator validator;
    if (!validator.validate(mod)) {
        std::cerr << "[-] IR verification failed!\n";
        for (const auto& e : validator.get_errors())
            std::cerr << "  - " << e.function_name << " [" << e.block_label << "]: " << e.message << "\n";
        fclose(file);
        return 1;
    }
    std::cout << "[+] IR verification passed!\n";

    if (opts.has_emit(EmitTarget::IR)) {
        const fs::path path = "output/out.ir";
        if (!ensure_dir(path.parent_path())) { fclose(file); return 1; }
        std::ofstream out(path);
        IRPrinter(out).print(mod);
        std::cout << "[+] Non-SSA IR saved to " << path << "\n";
    }


    CFGBuilder().build(mod);

    if (opts.has_emit(EmitTarget::CFG)) {
        if (!ensure_dir("output/dot")) { fclose(file); return 1; }
        for (auto& fn : mod.functions) {
            const fs::path path = "output/dot/cfg_" + fn->name + ".dot";
            std::ofstream out(path);
            if (out.is_open()) {
                CFGPrinter(out).print_fn(*fn);
                std::cout << "[+] CFG for @" << fn->name << " saved to " << path << "\n";
            }
        }
    }

    const bool default_mode = opts.emit_targets.empty();
    const bool need_llvm    = opts.has_emit(EmitTarget::LLVM) || default_mode;
    const bool need_ssa     = opts.has_emit(EmitTarget::SSA)  || need_llvm;
    const bool need_dom     = opts.has_emit(EmitTarget::DOM)
                           || opts.has_emit(EmitTarget::FDOM)
                           || need_ssa;

    if (need_dom) {
        if (!ensure_dir("output/dot")) { fclose(file); return 1; }

        for (auto& fn : mod.functions) {
            DomTree dom;
            dom.build(*fn);

            if (opts.has_emit(EmitTarget::DOM)) {
                const fs::path path = "output/dot/dom_" + fn->name + ".dot";
                std::ofstream out(path);
                if (out.is_open()) {
                    DomTreePrinter(out).print(*fn, dom);
                    std::cout << "[+] Dominator tree for @" << fn->name << " saved to " << path << "\n";
                }
            }

            DomFronts fronts;
            fronts.build(*fn, dom);

            if (opts.has_emit(EmitTarget::FDOM)) {
                const fs::path path = "output/dot/fdom_" + fn->name + ".dot";
                std::ofstream out(path);
                if (out.is_open()) {
                    DomFrontPrinter(out).print(*fn, fronts);
                    std::cout << "[+] Dominance frontiers for @" << fn->name << " saved to " << path << "\n";
                }
            }

            if (need_ssa) {
                Mem2Reg mem2reg;
                if (mem2reg.run(*fn, dom, fronts))
                    std::cout << "[+] Mem2Reg: promoted @" << fn->name << " to SSA\n";

                if (opts.opt_level >= OptLevel::O1) {
                    AlgebraicSimplify as;
                    if (as.run(*fn))
                        std::cout << "[+] ExprSimplification: simplified @" << fn->name << "\n";

                    ConstFold cf;
                    if (cf.run(*fn))
                        std::cout << "[+] ConstFold: folded constants in @" << fn->name << "\n";

                    DCE dce;
                    if (dce.run(*fn))
                        std::cout << "[+] DCE: eliminated dead instructions in @" << fn->name << "\n";
                }
            }
        }
    }

    if (opts.has_emit(EmitTarget::SSA)) {
        const fs::path path = "output/out.ssa";
        if (!ensure_dir(path.parent_path())) { fclose(file); return 1; }
        std::ofstream out(path);
        IRPrinter(out).print(mod);
        std::cout << "[+] SSA IR saved to " << path << "\n";
    }

    if (opts.has_emit(EmitTarget::LOOPS))
        std::cout << "[!] Loop tree emit: not yet implemented\n";

    if (need_llvm) {
        const fs::path ll_path = "output/out.ll";
        if (!ensure_dir(ll_path.parent_path())) { fclose(file); return 1; }

        {
            std::ofstream out(ll_path);
            if (!out.is_open()) {
                std::cerr << "senna: cannot open " << ll_path << " for writing\n";
                fclose(file);
                return 1;
            }
            LLVMEmitter(out).emit(mod);
        }
        std::cout << "[+] LLVM IR saved to " << ll_path << "\n";

        const fs::path exe = "output" / fs::path(opts.input_file).stem();
        const std::string cmd = "clang -O0 " + ll_path.string() + " -o " + exe.string();
        std::cout << "[✱] " << cmd << "\n";

        int ret = std::system(cmd.c_str());
        if (ret == 127) {
            std::cerr << "[-] clang not found\n";
            fclose(file);
            return 1;
        } else if (ret != 0) {
            std::cerr << "[-] clang compilation failed\n";
            fclose(file);
            return 1;
        }

        std::cout << "[+] Executable: " << exe << "\n";

        if (default_mode) {
            fs::remove(ll_path);
            std::cout << "[✱] Removed intermediate " << ll_path << "\n";
        }
    }

    fclose(file);
    return 0;
}