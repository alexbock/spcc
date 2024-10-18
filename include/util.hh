#ifndef SPCC_UTIL_HH
#define SPCC_UTIL_HH

#include <cstring>
#include <cstdint>
#include <cassert>
#include <string>
#include <string_view>

namespace util {
    inline std::string ltrim(const std::string& s) {
        auto start = s.find_first_not_of(' ');
        if (start != std::string::npos) return s.substr(start);
        else return s;
    }

    inline std::string rtrim(const std::string& s) {
        auto end = s.find_last_not_of(' ');
        if (end != std::string::npos) return s.substr(0, end + 1);
        else return s;
    }

    template<typename T>
    inline auto reverse_adaptor(T& t) {
        class reverse_impl {
        public:
            reverse_impl(T& t) : t(t) { }
            auto begin() { return t.rbegin(); }
            auto end() { return t.rend(); }
        private:
            T& t;
        } result(t);
        return result;
    }

    inline std::uintmax_t calculate_unsigned_max(unsigned bits) {
        std::uintmax_t result = 0;
        for (unsigned i = 0; i < bits; ++i) {
            result |= 1 << i;
        }
        return result;
    }
}

#endif
