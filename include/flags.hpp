#pragma once

#include <string>
#include <vector>
#include <getopt.h>
#include <iostream>
#include <unistd.h>


enum class EmitTarget {
    AST,
    IR,
};

struct CliOptions {
    std::string              input_file;
    std::vector<EmitTarget>  emit_targets;

    bool has_emit(EmitTarget t) const {
        for (const auto& e : emit_targets)
            if (e == t) return true;
        return false;
    }
};

namespace clinterface {

inline std::vector<std::string> split_csv(const std::string& s) {
    std::vector<std::string> ret;
    
    {
        std::stringstream ss(s);
        std::string token;

        while (std::getline(ss, token, ','))
            if (!token.empty()) ret.push_back(token);

    }

    return ret;
}
    

const bool use_color = isatty(STDERR_FILENO);

const char* bold  = use_color ? "\033[1m" : "";
const char* reset = use_color ? "\033[0m" : "";

static void print_usage(const char* name) {
    std::cerr 
    << bold << "Usage:" << reset << "\n"
    << "  " << name << " <input_file.sn> [options]\n"
    << "\n"
    << bold << "Options:" << reset << "\n"
    << "  " << bold << "--emit=<targets>" << reset << "    Comma-separated list of emit targets:\n"
    << "                        ast    Dump AST as Graphviz .dot file\n"
    << "                        ir     Dump intermediate representation\n"
    << "  " << bold << "-h, --help" << reset << "          Manual message\n\n";
}

} // namespace clinterface


inline CliOptions parse_args(int argc, char** argv) {
    CliOptions opts;

    static const option long_opts[] = {
        {"emit", required_argument, nullptr, 'e'},
        {"help", no_argument,       nullptr, 'h'},
        {nullptr, 0,                nullptr,  0 }
    };

    int c, idx;
    while ((c = getopt_long(argc, argv, "e:h", long_opts, &idx)) != -1) {
        switch (c) {
            case 'e':
                for (const auto& t : clinterface::split_csv(optarg)) {
                    if      (t == "ast") opts.emit_targets.push_back(EmitTarget::AST);
                    else if (t == "ir")  opts.emit_targets.push_back(EmitTarget::IR);
                    else {
                        std::cerr << "senna: unknown emit target '" << t << "'\n";
                        clinterface::print_usage(argv[0]);
                        exit(1);
                    }
                }
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