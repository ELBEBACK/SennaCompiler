#pragma once

#include <string>
#include <vector>
#include <getopt.h>
#include <iostream>
#include <unistd.h>
#include <sstream>


enum class EmitTarget {
    AST,
    IR,
    CFG,
    DOM,
    FDOM,
    SSA,
    LOOPS,
    LLVM,
};

enum class OptLevel {
    O0,
    O1,
};

struct CliOptions {
    std::string             input_file;
    std::vector<EmitTarget> emit_targets;
    OptLevel                opt_level = OptLevel::O1;
    bool                    explain   = false;

    bool has_emit(EmitTarget t) const {
        for (const auto& e : emit_targets)
            if (e == t) return true;
        return false;
    }
};

namespace clinterface {

inline std::vector<std::string> split_csv(const std::string& s) {
    std::vector<std::string> ret;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ','))
        if (!token.empty()) ret.push_back(token);
    return ret;
}

inline const bool use_color = isatty(STDERR_FILENO);
inline const char* bold  = use_color ? "\033[1m" : "";
inline const char* reset = use_color ? "\033[0m" : "";

static void print_usage(const char* name) {
    std::cerr
    << bold << "\nUsage:" << reset << "\n"
    << "  " << name << " <input_file.sn> [options]\n"
    << "\n"
    << bold << "Options:" << reset << "\n"
    << "  " << bold << "--emit=<targets>" << reset << "    Comma-separated list of emit targets:\n"
    << "                        ast    AST as Graphviz DOT\n"
    << "                        ir     Three-address IR before SSA\n"
    << "                        cfg    CFG as Graphviz DOT\n"
    << "                        dom    Dominator tree as Graphviz DOT\n"
    << "                        fdom   Dominance frontiers as Graphviz DOT\n"
    << "                        ssa    IR after mem2reg, in SSA form\n"
    << "                        loops  Loop nesting tree as Graphviz DOT\n"
    << "                        llvm   LLVM IR text, ready for clang\n"
    << "  " << bold << "-O<level>" << reset << "           Optimisation level (default: 1):\n"
    << "                        0      No optimisations\n"
    << "                        1      DCE; constant folding-propagation\n"
    << "  " << bold << "--explain" << reset << "           Trace pass decisions to stderr:\n"
    << "                        domtree  — dom-set changes per iteration, idom selection\n"
    << "                        fdom     — DF walk steps and stop conditions\n"
    << "                        phi      — IDF worklist expansion per variable\n"
    << "  " << bold << "-h, --help" << reset << "          Print this message\n\n";
}

}


inline CliOptions parse_args(int argc, char** argv) {
    CliOptions opts;

    static const option long_opts[] = {
        {"emit",    required_argument, nullptr, 'e'},
        {"explain", no_argument,       nullptr, 'x'},
        {"help",    no_argument,       nullptr, 'h'},
        {nullptr,   0,                 nullptr,  0 }
    };

    int c, idx;
    while ((c = getopt_long(argc, argv, "e:hO:x", long_opts, &idx)) != -1) {
        switch (c) {
            case 'e':
                for (const auto& t : clinterface::split_csv(optarg)) {
                    if      (t == "ast")   opts.emit_targets.push_back(EmitTarget::AST);
                    else if (t == "ir")    opts.emit_targets.push_back(EmitTarget::IR);
                    else if (t == "cfg")   opts.emit_targets.push_back(EmitTarget::CFG);
                    else if (t == "dom")   opts.emit_targets.push_back(EmitTarget::DOM);
                    else if (t == "fdom")  opts.emit_targets.push_back(EmitTarget::FDOM);
                    else if (t == "ssa")   opts.emit_targets.push_back(EmitTarget::SSA);
                    else if (t == "loops") opts.emit_targets.push_back(EmitTarget::LOOPS);
                    else if (t == "llvm")  opts.emit_targets.push_back(EmitTarget::LLVM);
                    else {
                        std::cerr << "senna: unknown emit target '" << t << "'\n";
                        clinterface::print_usage(argv[0]);
                        exit(1);
                    }
                }
                break;

            case 'O':
                if (std::string(optarg) == "0")
                    opts.opt_level = OptLevel::O0;
                else if (std::string(optarg) == "1")
                    opts.opt_level = OptLevel::O1;
                else {
                    std::cerr << "senna: unknown optimisation level '-O" << optarg << "'\n";
                    clinterface::print_usage(argv[0]);
                    exit(1);
                }
                break;

            case 'x':
                opts.explain = true;
                break;

            case 'h':
                clinterface::print_usage(argv[0]);
                exit(0);

            case '?':
                clinterface::print_usage(argv[0]);
                exit(1);
        }
    }

    if (optind >= argc) {
        std::cerr << "senna: no input file\n";
        clinterface::print_usage(argv[0]);
        exit(1);
    }

    opts.input_file = argv[optind];
    return opts;
}
