#include "test.hh"
#include "buffer.hh"
#include "location.hh"
#include "translator.hh"
#include "utf8.hh"

#include <iostream>
#include <cassert>
#include <cstdlib>

static void run_translator_tests();
static void run_utf8_tests();

void run_tests() {
#ifdef NDEBUG
    std::cerr << "tests are disabled due to NDEBUG\n";
    std::exit(EXIT_FAILURE);
#else
    run_translator_tests();
    run_utf8_tests();
#endif
    std::cerr << "internal tests passed\n";
}

#ifndef NDEBUG
void run_translator_tests() {
    std::cerr << "running translator tests\n";
}

void run_utf8_tests() {
    std::cerr << "running UTF-8 tests\n";
}
#endif
