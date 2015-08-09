#pragma once

#include <stdexcept>
#include <cstdint>
#include <string>

namespace utf8 {
    class invalid_utf8_error : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    bool is_ascii(char c);
    bool is_leader(char c);
    bool is_continuation(char c);
    unsigned measure_code_point(const std::string& str);
    std::uint32_t code_point_to_utf32(const std::string& str);
    std::string utf32_to_ucn(std::uint32_t u);
    std::size_t count_code_points_before(const std::string&, std::size_t);
}
