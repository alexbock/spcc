#include "diagnostic.hh"
#include "buffer.hh"
#include "location.hh"
#include "pp.hh"
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

    buffer b{"file.c", "\tfo\to€bar"};
    std::cout << "original:\n" << b.data << "---\n---\n";
    buffer b_pp1 = perform_pp_phase1(b);
    std::cout << "phase1:\n" << b_pp1.data << "---\n---\n";
    buffer b_pp2 = perform_pp_phase2(b_pp1);
    std::cout << "phase2:\n" << b_pp2.data << "---\n---\n";
}
