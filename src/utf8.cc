#include "utf8.hh"

#include <cassert>
#include <climits>
#include <sstream>
#include <ios>

inline static bool is_bit_set(char c, unsigned bit) {
    return c & (1U << bit);
}

// testing suggests that a lookup table could
// be 2-3x faster even at -O3
inline static unsigned count_set_high_bits(char c) {
    unsigned set = 0;
    for (unsigned i = CHAR_BIT; i != 0; --i) {
        if (c & 1U << (i - 1U)) ++set;
        else break;
    }
    return set;
}

namespace {
    class bit_cat {
    public:
        void push(unsigned char c, unsigned bad_high_bit_count) {
            c <<= bad_high_bit_count;
            c >>= bad_high_bit_count;
            value <<= (CHAR_BIT - bad_high_bit_count);
            value |= static_cast<std::uint32_t>(c);
        }

        uint32_t value = 0;
    };
}

namespace utf8 {
    bool is_ascii(char c) {
        return !is_bit_set(c, CHAR_BIT - 1);
    }

    bool is_leader(char c) {
        return is_bit_set(c, CHAR_BIT - 1) && is_bit_set(c, CHAR_BIT - 2);
    }

    bool is_continuation(char c) {
        return is_bit_set(c, CHAR_BIT - 1) && !is_bit_set(c, CHAR_BIT - 2);
    }

    unsigned measure_code_point(const std::string& str) {
        assert(!str.empty());
        if (is_ascii(str[0])) return 1;
        else if (is_leader(str[0])) return count_set_high_bits(str[0]);
        else throw invalid_utf8_error("stray UTF-8 continuation byte");
    }

    std::uint32_t code_point_to_utf32(const std::string& str) {
        assert(!str.empty());
        if (is_ascii(str[0])) return str[0];
        else if (is_leader(str[0])) {
            auto count = count_set_high_bits(str[0]);
            bit_cat bc;
            bc.push(str[0], count + 1);
            for (unsigned i = 1; i < count; ++i) {
                if (i >= str.size() || !is_continuation(str[i])) {
                    throw invalid_utf8_error("incomplete UTF-8 sequence");
                }
                bc.push(str[i], 2);
            }
            return bc.value;
        } else throw invalid_utf8_error("stray UTF-8 continuation byte");
    }

    std::string utf32_to_ucn(std::uint32_t u) {
        std::stringstream stream;
        stream << std::hex << std::uppercase << u;
        std::string result = stream.str();
        auto width = result.size() > 4 ? 8U : 4U;
        std::string prefix = (width == 8) ? "\\U" : "\\u";
        return prefix + std::string(width - result.size(), '0') + result;
    }
}
