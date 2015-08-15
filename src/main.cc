#include "options.hh"
#include "platform.hh"
#include "test.hh"
#include "diagnostic.hh"

#include "pp.hh"
#include "buffer.hh"
#include "pp_token.hh"
#include "color.hh"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

options program_options;

void debug_file(const std::string& filename);

int main(int argc, char** argv) {
    parse_options(argc, argv);
    if (program_options.parse_error) {
        std::exit(EXIT_FAILURE);
    } else if (program_options.version_requested) {
        std::cerr << "spcc -- simple C compiler\n";
        std::cerr << "copyright (c) 2015 Alexander Bock\n";
        std::cerr << "version: " PROGRAM_VERSION << "\n";
        std::cerr << "platform: " PLATFORM_NAME << "\n";
    } else if (program_options.help_requested) {
        std::cerr << "not yet implemented, sorry\n";
    } else if (program_options.run_internal_tests) {
        run_tests();
    } else if (program_options.files.empty()) {
        diagnose(diagnostic_id::no_input_files, {});
    } else {
        for (const auto& file : program_options.files) {
            debug_file(file);
        }
    }
}

void debug_dump_buffer(buffer& buf) {
    std::cerr << "@@@ " + buf.name + " @@@\n";
    std::cerr << buf.data << "@@@\n@@@\n";
}

void debug_dump_tokens(const std::vector<pp_token>& tokens) {
    if (program_options.enable_color) {
        for (const auto& token : tokens) {
            set_color(color::blue);
            std::cout << "•";
            set_color(color::standard);
            std::cout << token.spelling;

        }
    } else {
        for (const auto& token : tokens) {
            std::cout << "[" << token.spelling << "] ";
        }
    }
}

void debug_file(const std::string& filename) {
    std::ifstream file{filename.c_str()};
    if (!file.good()) {
        diagnose(diagnostic_id::cannot_open_file, {}, filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    buffer buf{filename, ss.str()};
    ss.str({});
    debug_dump_buffer(buf);
    auto p1 = perform_pp_phase1(buf);
    debug_dump_buffer(p1);
    auto p2 = perform_pp_phase2(p1);
    debug_dump_buffer(p2);
    auto p3 = perform_pp_phase3(p2);
    debug_dump_tokens(p3);
}
