#ifndef SPCC_UTF8_HH
#define SPCC_UTF8_HH

#include "string_view.hh"
#include "optional.hh"

#include <cstddef>
#include <cstdint>

namespace utf8 {
    bool is_ascii(char);
    bool is_leader(char);
    bool is_continuation(char);
    optional<std::size_t> measure_code_point(string_view);
    optional<std::uint32_t> code_point_to_utf32(string_view);
    std::string utf32_to_ucn(std::uint32_t);
}

#endif
