#include "options.hh"
#include "platform.hh"
#include "test.hh"
#include "diagnostic.hh"

#include <cstdlib>
#include <iostream>

options program_options;

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
        // TODO run
    }
}
