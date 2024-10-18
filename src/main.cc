#include "options.hh"
#include "diagnostic.hh"
#include "buffer.hh"
#include "pp.hh"
#include "test.hh"
#include "util.hh"
#include "platform.hh"
#include "parser.hh"
#include "declarator.hh"
#include "decl_spec.hh"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <print>

using diagnostic::diagnose;
using namespace platform::stream;

static void show_help();
static void show_version();
static void process_input_files();
static void debug_parse();
static void debug_scratch();

int main(int argc, char** argv) {
    options::parse(argc, argv);
    switch (options::state.mode) {
        case options::run_mode::show_version:
            show_version();
            break;
        case options::run_mode::show_help:
            show_help();
            break;
        case options::run_mode::run_tests:
            test::run_tests();
            break;
        case options::run_mode::normal:
            process_input_files();
            break;
        case options::run_mode::option_parsing_error:
            break;
        case options::run_mode::dump_config:
            options::dump();
            break;
        case options::run_mode::debug_parse_declarator:
        case options::run_mode::debug_parse_expr:
            debug_parse();
            break;
        case options::run_mode::debug_scratch:
            debug_scratch();
            break;
    }
    return options::state.exit_code;
}

void show_help() {
    std::println("help is not yet implemented");
}

void show_version() {
    std::println("spcc 0.1 (c) 2016-2024 Alexander Bock\n");
}

static void debug_dump_tokens(const std::vector<token>& tokens) {
    bool first = false;
    for (const auto tok : tokens) {
        if (!first) {
            set_color(stdout, color::blue);
            std::print(".");
            reset_attributes(stdout);
        }
        first = false;

        std::print("{}", tok.spelling);
    }
}

void process_input_files() {
    if (options::state.input_filenames.empty()) {
        diagnose(diagnostic::id::no_input_files, {});
    }
    for (const auto& filename : options::state.input_filenames) {
        if (!util::ends_with(filename, ".c")) {
            diagnose(diagnostic::id::input_file_not_dot_c, {}, filename);
        }
        std::ifstream file{filename};
        if (!file.good()) {
            diagnose(diagnostic::id::cannot_open_file, {}, filename);
            continue;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        auto data = ss.str();
        ss.clear();

        auto buf = std::make_unique<raw_buffer>(filename, data);
        auto post_p1 = pp::perform_phase_one(std::move(buf));
        auto post_p2 = pp::perform_phase_two(std::move(post_p1));
        auto tokens = pp::perform_phase_three(*post_p2);
        pp::phase_four_manager p4m(std::move(post_p2), std::move(tokens));
        tokens = p4m.process();
        pp::remove_whitespace(tokens);
        pp::buffer_ptrs extra_buffers;
        tokens = pp::perform_phase_six(std::move(tokens), extra_buffers);
        tokens = pp::perform_phase_seven(tokens);
        std::println("");
        debug_dump_tokens(tokens);
        std::println("");
    }
}

void debug_parse() {
    bool is_declarator = true;
    if (options::state.mode == options::run_mode::debug_parse_expr) {
        is_declarator = false;
    }
    const auto& data = options::state.debug_string_to_parse;
    auto buf = std::make_unique<raw_buffer>("<debug>", data);
    auto post_p1 = pp::perform_phase_one(std::move(buf));
    auto post_p2 = pp::perform_phase_two(std::move(post_p1));
    auto tokens = pp::perform_phase_three(*post_p2);
    pp::phase_four_manager p4m(std::move(post_p2), std::move(tokens));
    tokens = p4m.process();
    pp::remove_whitespace(tokens);
    pp::buffer_ptrs extra_buffers;
    tokens = pp::perform_phase_six(std::move(tokens), extra_buffers);
    tokens = pp::perform_phase_seven(tokens);

    parse::parser p{tokens};
    p.push_ruleset(is_declarator);
    auto node = p.parse(0);
    p.pop_ruleset();
    node->dump();
}

void debug_scratch() {
    const auto& data = options::state.debug_string_to_parse;
    auto buf = std::make_unique<raw_buffer>("<debug>", data);
    auto post_p1 = pp::perform_phase_one(std::move(buf));
    auto post_p2 = pp::perform_phase_two(std::move(post_p1));
    auto tokens = pp::perform_phase_three(*post_p2);
    pp::phase_four_manager p4m(std::move(post_p2), std::move(tokens));
    tokens = p4m.process();
    pp::remove_whitespace(tokens);
    pp::buffer_ptrs extra_buffers;
    tokens = pp::perform_phase_six(std::move(tokens), extra_buffers);
    tokens = pp::perform_phase_seven(tokens);

    parse::parser p{tokens};
    auto ds = parse::parse_decl_spec(p);
    auto idl = parse::parse_init_declarator_list(p);
    for (const auto& id : idl) {
        id.declarator->dump();
        if (id.init) {
            std::print(stderr, " = ");
            id.init->dump();
        }
    }
    int bp = 0;
}
