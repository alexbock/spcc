#include "options.hh"
#include "util.hh"
#include "diagnostic.hh"

#include <utility>
#include <cassert>
#include <map>
#include <cstdlib>
#include <iostream>
#include <optional>

using util::starts_with;
using diagnostic::diagnose;

options::config options::state;

namespace options {
    struct option {
        std::string short_form; // optional
        std::string long_form; // mandatory
        void (*handler)(std::string, std::optional<std::string> arg);
        bool allow_arg = false;
        bool require_arg = false;
        std::string description;
        std::string usage;
    };

    using option_id = std::pair<std::string, std::string>;

    std::map<std::string, option> options;
    std::map<std::string, std::string> short_to_long;

    void register_option(option opt) {
        assert(!opt.require_arg || opt.allow_arg);
        if (!opt.short_form.empty()) {
            short_to_long[opt.short_form] = opt.long_form;
        }
        options[opt.long_form] = std::move(opt);
    }

    void handle_help(std::string, std::optional<std::string>) {
        state.mode = run_mode::show_help;
    }

    void handle_version(std::string, std::optional<std::string>) {
        state.mode = run_mode::show_version;
    }

    void handle_test(std::string, std::optional<std::string>) {
        state.mode = run_mode::run_tests;
    }

    template<unsigned size_info::*Size>
    void handle_any_size_option(std::string opt, std::optional<std::string> arg) {
        auto sz = std::atoi(arg->c_str());
        if (sz <= 0) {
            diagnose(diagnostic::id::invalid_option, {},
                     opt, "invalid argument");
            state.mode = run_mode::option_parsing_error;
            return;
        }
        state.sizes.*Size = sz;
    }

    void handle_char(std::string opt, std::optional<std::string> arg) {
        if (*arg == "signed") state.is_char_signed = true;
        else if (*arg == "unsigned") state.is_char_signed = false;
        else {
            diagnose(diagnostic::id::invalid_option, {},
                     opt, "argument must be 'signed' or 'unsigned'");
            state.mode = run_mode::option_parsing_error;
        }
    }

    void handle_dump_config(std::string, std::optional<std::string>) {
        state.mode = run_mode::dump_config;
    }

    void handle_parse_declarator(std::string, std::optional<std::string> arg) {
        state.mode = run_mode::debug_parse_declarator;
        state.debug_string_to_parse = *arg + "\n";
    }

    void handle_parse_expr(std::string, std::optional<std::string> arg) {
        state.mode = run_mode::debug_parse_expr;
        state.debug_string_to_parse = *arg + "\n";
    }

    void handle_debug_scratch(std::string, std::optional<std::string> arg) {
        state.mode = run_mode::debug_scratch;
        if (arg) state.debug_string_to_parse = *arg + "\n";
    }

    void register_options() {
        assert(options.empty() && "options already registered");
        register_option({
            {}, "help",
            handle_help,
            false, false,
            "show help",
            {}
        });
        register_option({
            {}, "version",
            handle_version,
            false, false,
            "show version information",
            {}
        });
        register_option({
            {}, "test",
            handle_test,
            false, false,
            "run unit tests",
            {}
        });
        register_option({
            {}, "bits-per-byte",
            handle_any_size_option<&size_info::bits_per_byte>,
            true, true,
            "configure the number of bits per byte (must match limits.h)",
            "--bits-per-byte=n"
        });
        register_option({
            {}, "size-bytes",
            handle_any_size_option<&size_info::size_bytes>,
            true, true,
            "configure the size of std::size_t (must match stddef.h)",
            "--size-bytes=n"
        });
        register_option({
            {}, "short-bytes",
            handle_any_size_option<&size_info::short_bytes>,
            true, true,
            "configure the size of the short type",
            "--short-bytes=n"
        });
        register_option({
            {}, "int-bytes",
            handle_any_size_option<&size_info::int_bytes>,
            true, true,
            "configure the size of the int type"
            "--int-bytes=n"
        });
        register_option({
            {}, "long-bytes",
            handle_any_size_option<&size_info::long_bytes>,
            true, true,
            "configure the size of the long type",
            "--long-bytes=n"
        });
        register_option({
            {}, "long-long-bytes",
            handle_any_size_option<&size_info::long_long_bytes>,
            true, true,
            "configure the size of the long long type",
            "--long-long-bytes=n"
        });
        register_option({
            {}, "char",
            handle_char,
            true, true,
            "configure the signedness of the plain char type",
            "--char=signed|unsigned"
        });
        register_option({
            {}, "dump-config",
            handle_dump_config,
            false, false,
            "dump configuration",
            {}
        });
        register_option({
            {}, "parse-declarator",
            handle_parse_declarator,
            true, true,
            "(debug) parse a declarator",
            {}
        });
        register_option({
            {}, "parse-expr",
            handle_parse_expr,
            true, true,
            "(debug) parse an expression",
            {}
        });
        register_option({
            {}, "debug-scratch",
            handle_debug_scratch,
            true, false,
            "(debug) scratch space",
            {}
        });
    }

    void validate_sizes();
}

void options::parse(int argc, char** argv) {
    register_options();
    std::vector<std::string> args(argv + 1, argv + argc);
    for (auto& arg : args) {
        if (state.mode == run_mode::option_parsing_error) return;
        if (starts_with(arg, "-")) {
            std::string long_form = "";
            std::optional<std::string> opt_arg;
            if (starts_with(arg, "--")) {
                long_form = arg.substr(2);
                long_form = long_form.substr(0, long_form.find('='));
                if (arg.find('=') != std::string::npos) {
                    opt_arg = arg.substr(arg.find('=') + 1);
                }
            } else {
                auto short_form = arg.substr(1, 1);
                auto it = short_to_long.find(short_form);
                if (it != short_to_long.end()) {
                    long_form = it->second;
                    if (arg.size() > 2) {
                        opt_arg = arg.substr(2);
                    }
                }
            }
            auto it = options.find(long_form);
            if (it != options.end()) {
                if (it->second.require_arg && !opt_arg) {
                    diagnose(diagnostic::id::invalid_option, {},
                             arg, "missing argument");
                    state.mode = run_mode::option_parsing_error;
                    return;
                } else if (!it->second.allow_arg && opt_arg) {
                    diagnose(diagnostic::id::invalid_option, {},
                             arg, "option does not take arguments");
                    state.mode = run_mode::option_parsing_error;
                    return;
                }
                it->second.handler(arg, opt_arg);
            } else {
                diagnose(diagnostic::id::invalid_option, {},
                         arg, "unknown option name");
                state.mode = run_mode::option_parsing_error;
                return;
            }
        } else {
            state.input_filenames.push_back(std::move(arg));
        }
    }
    validate_sizes();
}

void options::validate_sizes() {
    const auto& sizes = state.sizes;
    bool good = true;
    std::string error;
    const auto bpb = sizes.bits_per_byte;
    auto check_type = [&](unsigned bytes, std::uintmax_t min_max,
                          std::string type_name, std::string option_name) {
        if (!good) return;
        auto actual_max = util::calculate_unsigned_max(bytes * bpb);
        if (actual_max < min_max) {
            error = "unsigned " + type_name + " must be able to store";
            error += " " + std::to_string(min_max);
            error += " (--" + option_name + "-bytes=";
            error += std::to_string(bytes) + " yields a maximum value of ";
            error += std::to_string(actual_max) + ")";
            good = false;
        }
    };
    if (sizes.bits_per_byte < 8) {
        error = "byte must have at least 8 bits";
        error += " (--bits-per-byte=";
        error += std::to_string(sizes.bits_per_byte) + ")";
        good = false;
    }
    check_type(sizes.short_bytes, 65535U, "short", "short");
    check_type(sizes.int_bytes, 65535U, "int", "int");
    check_type(sizes.long_bytes, 4294967295U, "long", "long");
    check_type(sizes.long_long_bytes, 18446744073709551615U,
               "long long", "long-long");
    if (!good) {
        diagnose(diagnostic::id::invalid_size, {}, error);
        state.mode = run_mode::option_parsing_error;
    }
}

void options::dump() {
    if (!state.input_filenames.empty()) {
        std::cout << "input filenames:\n";
        for (const auto& filename : state.input_filenames) {
            std::cout << "\t" << filename << "\n";
        }
    }

    std::cout << "plain char: ";
    if (state.is_char_signed) std::cout << "signed\n";
    else std::cout << "unsigned\n";

    std::cout << "bits per byte: " << state.sizes.bits_per_byte << "\n";
    std::cout << "sizeof(short): " << state.sizes.short_bytes << "\n";
    std::cout << "sizeof(int): " << state.sizes.int_bytes << "\n";
    std::cout << "sizeof(long): " << state.sizes.long_bytes << "\n";
    std::cout << "sizeof(long long): " << state.sizes.long_long_bytes << "\n";
}
