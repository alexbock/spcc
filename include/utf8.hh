#ifndef SPCC_UTF8_HH
#define SPCC_UTF8_HH

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace utf8 {
    bool is_ascii(unsigned char);
    bool is_leader(unsigned char);
    bool is_continuation(unsigned char);
    std::optional<std::size_t> measure_code_point(std::string_view);
    std::optional<std::uint32_t> code_point_to_utf32(std::string_view);
    std::string utf32_to_ucn(std::uint32_t);
}

#endif
