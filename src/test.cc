#include "test.hh"
#include "buffer.hh"
#include "location.hh"
#include "translator.hh"
#include "utf8.hh"
#include "pp.hh"

#include <iostream>
#include <cassert>
#include <cstdlib>

static void run_translator_tests();
static void run_utf8_tests();
static void run_pp_phase1_tests();
static void run_pp_phase2_tests();

void run_tests() {
#ifdef NDEBUG
    std::cerr << "tests are disabled due to NDEBUG\n";
    std::exit(EXIT_FAILURE);
#else
    run_translator_tests();
    run_utf8_tests();
    run_pp_phase1_tests();
    run_pp_phase2_tests();
#endif
    std::cerr << "internal tests passed\n";
}

#ifndef NDEBUG
void run_translator_tests() {
    std::cerr << "running translator tests\n";
    buffer b1{"", "this is a test"};
    buffer b2{"", ""};
    auto& t = *(b2.src = std::make_unique<translator>(b1, b2));
    t.propagate(5);
    t.replace(2, "was");
    t.propagate(1);
    t.replace(1, "an");
    t.propagate(1);
    t.insert("excellent ");
    t.propagate(4);
    assert(b2.data == "this was an excellent test");
    assert(t.translate_dst_to_src(0) == 0);
    assert(t.translate_dst_to_src(1) == 1);
    assert(t.translate_dst_to_src(5) == 5);
    assert(t.translate_dst_to_src(7) == 5);
    assert(t.translate_dst_to_src(9) == 8);
    assert(t.translate_dst_to_src(10) == 8);
    assert(t.translate_dst_to_src(12) == 10);
    assert(t.translate_dst_to_src(22) == 10);
}

void run_utf8_tests() {
    std::cerr << "running UTF-8 tests\n";
    assert(utf8::is_ascii('a'));
    assert(utf8::is_ascii('~'));
    assert(utf8::is_ascii('z'));
    assert(utf8::is_ascii('E'));
    assert(utf8::is_ascii('4'));
    assert(utf8::is_ascii('\n'));
    assert(utf8::measure_code_point("a") == 1);
    assert(utf8::measure_code_point("xyz") == 1);
    assert(utf8::measure_code_point("\n") == 1);
    assert(utf8::measure_code_point("€") == 3);
    assert(utf8::code_point_to_utf32("€") == 0x20ACU);
    assert(utf8::utf32_to_ucn(0x20ACU) == "\\u20AC");
    assert(utf8::is_leader("€"[0]));
    assert(utf8::is_continuation("€"[1]));
    assert(utf8::is_continuation("€"[2]));
    assert(!utf8::is_leader("€"[1]));
}

void run_pp_phase1_tests() {
    std::cerr << "running preprocessor phase 1 tests\n";
    assert(!"TODO");
}

void run_pp_phase2_tests() {
    std::cerr << "running preprocessor phase 2 tests\n";
    assert(!"TODO");
}
#endif
