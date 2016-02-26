#include "utf8.hh"

#include <climits>
#include <cassert>
#include <sstream>
#include <ios>

static_assert(CHAR_BIT == 8, "UTF-8 encoding expects 8-bit chars");

static std::size_t active_high_bits[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8,
};

bool utf8::is_ascii(char c) {
    return c >> 7 == 0U;
}

bool utf8::is_leader(char c) {
    return c >> 6 == 0b11U;
}

bool utf8::is_continuation(char c) {
    return c >> 6 == 0b10U;
}

optional<std::size_t> utf8::measure_code_point(string_view str) {
    if (str.empty()) return {};
    else if (is_ascii(str[0])) return 1;
    else if (is_leader(str[0])) return active_high_bits[+str[0]];
    else return {};
}

optional<std::uint32_t> utf8::code_point_to_utf32(string_view str) {
    if (str.empty() || is_continuation(str[0])) return {};
    else if (is_ascii(str[0])) return str[0];
    const auto count = measure_code_point(str);
    const std::size_t init_size = count + 1;
    auto result = static_cast<std::uint32_t>(str[0]) << (32 - init_size);
    std::size_t pos = init_size;
    for (std::size_t i = 1; i < count; ++i) {
        result |= static_cast<std::uint32_t>(str[i]) << (32 - 6) >> pos;
        pos += 6;
    }
    return result;
}

std::string utf8::utf32_to_ucn(std::uint32_t u) {
    std::stringstream stream;
    stream << std::hex << std::uppercase << u;
    std::string result = stream.str();
    auto width = result.size() > 4U ? 8U : 4U;
    std::string prefix = (width == 8U) ? "\\U" : "\\u";
    return prefix + std::string(width - result.size(), '0') + result;
}
