#include "diagnostic.hh"
#include "buffer.hh"
#include "location.hh"
#include "pp.hh"

#include <iostream>

int main(int argc, char** argv) {
    buffer b{"file.c", "\tfo\to€bar"};
    std::cout << "original:\n" << b.data << "---\n---\n";
    buffer b_pp1 = perform_pp_phase1(b);
    std::cout << "phase1:\n" << b_pp1.data << "---\n---\n";
    buffer b_pp2 = perform_pp_phase2(b_pp1);
    std::cout << "phase2:\n" << b_pp2.data << "---\n---\n";

    // TODO measure width in UTF-8 code points for carets
    // TODO stop colors in Xcode
    // TODO unit tests
    // TODO phase 3
}
