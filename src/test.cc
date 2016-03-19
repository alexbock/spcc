#include "test.hh"
#include "buffer.hh"
#include "utf8.hh"
#include "pp.hh"
#include "platform.hh"

#include <iostream>
#include <memory>
#include <cstdio>

using namespace platform::stream;

static void run_derived_buffer_tests();
static void run_utf8_tests();
static void run_pp_regex_tests();

#define TEST(x) run_test(x, #x, __FILE__, __LINE__)

static void run_test(bool passed, const char* expr,
                     const char* file, int line) {
    std::string result;
    color col;
    if (passed) {
        result = "passed";
        col = color::green;
    } else {
        result = "failed";
        col = color::red;
    }
    set_color(stdout, col);
    std::cout << result << ": ";
    reset_attributes(stdout);
    set_style(stdout, style::bold);
    set_color(stdout, color::white);
    std::cout << expr;
    reset_attributes(stdout);
    std::cout << " (" << __FILE__ << ":" << __LINE__ << ")\n";
}

void test::run_tests() {
    run_derived_buffer_tests();
    run_utf8_tests();
    run_pp_regex_tests();
}

void run_derived_buffer_tests() {
    std::cout << "running derived buffer tests...\n";
    auto rb = std::make_unique<raw_buffer>("<test>", "this is a test");
    auto db = std::make_unique<derived_buffer>(std::move(rb));
    db->propagate(5);
    db->replace(2, "was");
    db->propagate(1);
    db->replace(1, "an");
    db->propagate(1);
    db->insert("excellent ");
    db->propagate(4);
    TEST(db->data() == "this was an excellent test");
    TEST(db->offset_in_original(0) == 0);
    TEST(db->offset_in_original(1) == 1);
    TEST(db->offset_in_original(5) == 5);
    TEST(db->offset_in_original(7) == 5);
    TEST(db->offset_in_original(9) == 8);
    TEST(db->offset_in_original(10) == 8);
    TEST(db->offset_in_original(12) == 10);
    TEST(db->offset_in_original(22) == 10);
}

void run_utf8_tests() {
    std::cout << "running UTF-8 tests...\n";
    TEST(utf8::is_ascii('a'));
    TEST(utf8::is_ascii('~'));
    TEST(utf8::is_ascii('z'));
    TEST(utf8::is_ascii('E'));
    TEST(utf8::is_ascii('4'));
    TEST(utf8::is_ascii('$'));
    TEST(utf8::is_ascii('\n'));
    TEST(*utf8::measure_code_point("a") == 1);
    TEST(*utf8::measure_code_point("$") == 1);
    TEST(*utf8::measure_code_point("xyz") == 1);
    TEST(*utf8::measure_code_point("\n") == 1);
    TEST(*utf8::measure_code_point("¢") == 2);
    TEST(*utf8::measure_code_point("€") == 3);
    TEST(*utf8::measure_code_point("𐍈") == 4);
    TEST(*utf8::code_point_to_utf32("€") == 0x20ACU);
    TEST(*utf8::code_point_to_utf32("¢") == 0x00A2U);
    TEST(*utf8::code_point_to_utf32("𐍈") == 0x00010348U);
    TEST(utf8::utf32_to_ucn(0x20ACU) == "\\u20AC");
    TEST(utf8::utf32_to_ucn(0x00A2U) == "\\u00A2");
    TEST(utf8::utf32_to_ucn(0x00010348U) == "\\U00010348");
    TEST(utf8::is_leader("€"[0]));
    TEST(utf8::is_continuation("€"[1]));
    TEST(utf8::is_continuation("€"[2]));
    TEST(!utf8::is_leader("€"[1]));
    TEST(!utf8::is_ascii("€"[0]));
    TEST(!utf8::is_ascii("€"[1]));
    TEST(!utf8::is_continuation("€"[0]));
    TEST(!utf8::is_continuation("a"[0]));
    TEST(!utf8::is_leader("a"[0]));
}

void run_pp_regex_tests() {
    std::cout << "running preprocessor regex tests...\n";
    TEST(std::regex_match("<foo.h>", pp::regex::header_name));
    TEST(std::regex_match("< bar/cat.hh >", pp::regex::header_name));
    TEST(std::regex_match("\"foo.h\"", pp::regex::header_name));
    TEST(std::regex_match("\" bar/cat.hh \"", pp::regex::header_name));
    TEST(!std::regex_match("< bar/cat.hh \"", pp::regex::header_name));
    TEST(std::regex_match("ab\\u1111c123_f", pp::regex::identifier));
    TEST(!std::regex_match("5abc", pp::regex::identifier));
    TEST(std::regex_match("5.6", pp::regex::pp_number));
    TEST(std::regex_match(".6.", pp::regex::pp_number));
    TEST(std::regex_match("123", pp::regex::pp_number));
    TEST(std::regex_match("123.456efg", pp::regex::pp_number));
    TEST(std::regex_match("5.6e+10", pp::regex::pp_number));
    TEST(!std::regex_match("five", pp::regex::pp_number));
    TEST(std::regex_match("#", pp::regex::punctuator));
    TEST(std::regex_match("// comment\n", pp::regex::space));
    TEST(std::regex_match("/*comment*/", pp::regex::space));
    TEST(std::regex_match("/*\n//\n*/", pp::regex::space));
    TEST(std::regex_match("/*comment\n", pp::regex::space));
    TEST(!std::regex_match("/*/*comment*/*/", pp::regex::space));
    TEST(std::regex_match("/*/*comment*/", pp::regex::space));
    TEST(std::regex_match("foo", pp::regex::identifier));
}