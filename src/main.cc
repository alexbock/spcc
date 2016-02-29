#include "options.hh"
#include "diagnostic.hh"
#include "buffer.hh"
#include "pp.hh"
#include "test.hh"
#include "util.hh"
#include "platform.hh"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>

using diagnostic::diagnose;
using namespace platform::stream;

static void show_help();
static void show_version();
static void process_input_files();

int main(int argc, char** argv) {
    //test::run_tests();
    options::parse(argc, argv);
    if (options::state.show_version) show_version();
    if (options::state.show_help) show_help();
    process_input_files();
    return options::state.exit_code;
}

void show_help() {
    std::cout << "help is not yet implemented\n";
}

void show_version() {
    std::cout << "spcc 0.1 (c) 2016 Alexander Bock\n";
}

static void debug_dump_tokens(const std::vector<token> tokens) {
    bool first = false;
    for (const auto tok : tokens) {
        if (!first) {
            set_color(stdout, color::blue);
            std::cout << "·";
            reset_attributes(stdout);
        }
        first = false;

        std::cout << tok.spelling;
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

        //std::cout << "@@@" << data << "@@@\n\n";
        auto buf = std::make_unique<raw_buffer>(filename, data);
        auto post_p1 = pp::perform_phase_one(std::move(buf));
        auto post_p2 = pp::perform_phase_two(std::move(post_p1));
        //std::cout << "@@@" << post_p2->data() << "@@@\n";
        auto tokens = pp::perform_phase_three(*post_p2);
        //debug_dump_tokens(tokens);
        //std::cout << "@@@\n";
        pp::phase_four_manager p4m(std::move(post_p2), std::move(tokens));
        tokens = p4m.process();
        //debug_dump_tokens(tokens);
        //std::cout << "@@@\n";
        //std::cout << "\n";
        for (auto tok : tokens) {
            if (tok.is(token::space)) std::cout << " ";
            else std::cout << tok.spelling;
        }
        std::cout << "\n";
    }
}
