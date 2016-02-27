#include "test.hh"
#include "buffer.hh"
#include "utf8.hh"
#include "pp.hh"

#include <iostream>
#include <memory>

static void run_derived_buffer_tests();
static void run_utf8_tests();
static void run_pp_regex_tests();

void test::run_tests() {
#ifndef NDEBUG
    run_derived_buffer_tests();
    run_utf8_tests();
    run_pp_regex_tests();
#else
    std::cerr << "tests disabled due to NDEBUG\n";
#endif
}

void run_derived_buffer_tests() {
    std::cerr << "=== running derived buffer tests\n";
    auto rb = std::make_unique<raw_buffer>("<test>", "this is a test");
    auto db = std::make_unique<derived_buffer>(std::move(rb));
    db->propagate(5);
    db->replace(2, "was");
    db->propagate(1);
    db->replace(1, "an");
    db->propagate(1);
    db->insert("excellent ");
    db->propagate(4);
    assert(db->data() == "this was an excellent test");
    assert(db->offset_in_original(0) == 0);
    assert(db->offset_in_original(1) == 1);
    assert(db->offset_in_original(5) == 5);
    assert(db->offset_in_original(7) == 5);
    assert(db->offset_in_original(9) == 8);
    assert(db->offset_in_original(10) == 8);
    assert(db->offset_in_original(12) == 10);
    assert(db->offset_in_original(22) == 10);
}

void run_utf8_tests() {
    std::cerr << "=== running UTF-8 tests\n";
    assert(utf8::is_ascii('a'));
    assert(utf8::is_ascii('~'));
    assert(utf8::is_ascii('z'));
    assert(utf8::is_ascii('E'));
    assert(utf8::is_ascii('4'));
    assert(utf8::is_ascii('$'));
    assert(utf8::is_ascii('\n'));
    assert(*utf8::measure_code_point("a") == 1);
    assert(*utf8::measure_code_point("$") == 1);
    assert(*utf8::measure_code_point("xyz") == 1);
    assert(*utf8::measure_code_point("\n") == 1);
    assert(*utf8::measure_code_point("¢") == 2);
    assert(*utf8::measure_code_point("€") == 3);
    assert(*utf8::measure_code_point("𐍈") == 4);
    assert(*utf8::code_point_to_utf32("€") == 0x20ACU);
    assert(*utf8::code_point_to_utf32("¢") == 0x00A2U);
    assert(*utf8::code_point_to_utf32("𐍈") == 0x00010348U);
    assert(utf8::utf32_to_ucn(0x20ACU) == "\\u20AC");
    assert(utf8::utf32_to_ucn(0x00A2U) == "\\u00A2");
    assert(utf8::utf32_to_ucn(0x00010348U) == "\\U00010348");
    assert(utf8::is_leader("€"[0]));
    assert(utf8::is_continuation("€"[1]));
    assert(utf8::is_continuation("€"[2]));
    assert(!utf8::is_leader("€"[1]));
    assert(!utf8::is_ascii("€"[0]));
    assert(!utf8::is_ascii("€"[1]));
    assert(!utf8::is_continuation("€"[0]));
    assert(!utf8::is_continuation("a"[0]));
    assert(!utf8::is_leader("a"[0]));
}

void run_pp_regex_tests() {
    std::cerr << "=== running preprocessor regex tests\n";
    assert(std::regex_match("<foo.h>", pp::regex::header_name));
    assert(std::regex_match("< bar/cat.hh >", pp::regex::header_name));
    assert(std::regex_match("\"foo.h\"", pp::regex::header_name));
    assert(std::regex_match("\" bar/cat.hh \"", pp::regex::header_name));
    assert(!std::regex_match("< bar/cat.hh \"", pp::regex::header_name));
    assert(std::regex_match("ab\\u1111c123_f", pp::regex::identifier));
    assert(!std::regex_match("5abc", pp::regex::identifier));
    assert(std::regex_match("5.6", pp::regex::pp_number));
    assert(std::regex_match(".6.", pp::regex::pp_number));
    assert(std::regex_match("123", pp::regex::pp_number));
    assert(std::regex_match("123.456efg", pp::regex::pp_number));
    assert(std::regex_match("5.6e+10", pp::regex::pp_number));
    assert(!std::regex_match("five", pp::regex::pp_number));
    assert(std::regex_match("#", pp::regex::punctuator));
    assert(std::regex_match("// comment\n", pp::regex::space));
    assert(std::regex_match("/*comment*/", pp::regex::space));
    assert(std::regex_match("/*comment\n", pp::regex::space));
    assert(!std::regex_match("/*/*comment*/*/", pp::regex::space));
    assert(std::regex_match("/*/*comment*/", pp::regex::space));
    assert(std::regex_match("foo", pp::regex::identifier));
}