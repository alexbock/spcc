#include "diagnostic.hh"
#include "options.hh"
#include "util.hh"

#include <map>
#include <string>
#include <functional>
#include <iostream>

struct option {
    std::string name;
    std::string argument;
    std::string prefix;
};

using option_handler = void(*)(options&, const option&);

static void option_test(options& opts, const option& opt) {
    std::cerr << "name: " << opt.name << "\n";
    std::cerr << "arg: " << opt.argument << "\n";
}

static std::map<std::string, option_handler> register_options() {
    return {
        { "--long-option-test", option_test },
        { "-`", option_test },
        { "--version", [](options& opts, const option&) {
            opts.version_requested = true;
        }},
        { "--help", [](options& opts, const option&) {
            opts.help_requested = true;
        }},
        { "--enable-color", [](options& opts, const option&) {
            opts.enable_color = true;
        }},
        { "--disable-color", [](options& opts, const option&) {
            opts.enable_color = false;
        }},
        { "--run-internal-tests", [](options& opts, const option&) {
            opts.run_internal_tests = true;
        }},
    };
}

void parse_options(int argc, char** argv) {
    auto handlers = register_options();
    options& opts = program_options;
    bool only_files = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        option opt;
        if (starts_with(arg, "--") && !only_files) {
            if (arg.size() == 2) {
                only_files = true;
                continue;
            }
            opt.prefix = "--";
            auto eq_pos = arg.find('=');
            if (eq_pos == std::string::npos) {
                opt.name = arg.substr(2);
#if 0
                if (i != argc - 1 && !starts_with(argv[i + 1], "-")) {
                    ++i;
                    opt.argument = argv[i];
                }
#endif
            } else {
                opt.name = arg.substr(2, eq_pos - 2);
                opt.argument = arg.substr(eq_pos + 1);
            }
        } else if (starts_with(arg, "-" ) && !only_files) {
            if (arg.size() == 1) {
                opts.input_from_stdin = true;
                continue;
            }
            opt.prefix = "-";
            opt.name = arg.substr(1, 1);
            opt.argument = arg.substr(2);
        } else {
            opts.files.push_back(arg);
            continue;
        }
        auto it = handlers.find(opt.prefix + opt.name);
        if (it == handlers.end()) {
            diagnose(diagnostic_id::opt_unrecognized, {}, arg);
            opts.parse_error = true;
            continue;
        }
        it->second(opts, opt);
    }
}
