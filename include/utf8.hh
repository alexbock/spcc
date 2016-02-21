#pragma once

#include <stdexcept>
#include <cstdint>
#include <string>
#include <experimental/string_view>

using std::experimental::string_view;

namespace utf8 {
    class invalid_utf8_error : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    bool is_ascii(char c);
    bool is_leader(char c);
    bool is_continuation(char c);
    unsigned measure_code_point(string_view str);
    std::uint32_t code_point_to_utf32(string_view str);
    std::string utf32_to_ucn(std::uint32_t u);
}
