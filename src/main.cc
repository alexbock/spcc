#include "options.hh"

#include <iostream>
#include <cstdlib>

static void show_help();
static void show_version();
static void process_input_files();

int main(int argc, char** argv) {
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

void process_input_files() {
    if (options::state.input_filenames.empty()) {
        // TODO use diagnostic system
        std::cerr << "error: no input files\n";
        options::state.exit_code = 1;
    }
    for (const auto& file : options::state.input_filenames) {
        // TODO
    }
}
