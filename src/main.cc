#include "options.hh"
#include "test.hh"

#include <cstdlib>
#include <iostream>

options program_options;

int main(int argc, char** argv) {
    program_options = parse_options(argc, argv);
    if (program_options.parse_error) {
        std::exit(EXIT_FAILURE);
    }
    if (program_options.run_internal_tests) {
        run_tests();
        std::exit(EXIT_SUCCESS);
    }
    // TODO
}
