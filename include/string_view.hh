#ifndef SPCC_STRING_VIEW_HH
#define SPCC_STRING_VIEW_HH

#include <string>
#include <cstddef>
#include <cstring>
#include <algorithm>

namespace meta {
    class string_view {
    public:
        string_view() : str{nullptr}, len{0} { }
        string_view(const std::string& s) : str{s.c_str()}, len{s.size()} { }
        string_view(const char* s) : str{s}, len{std::strlen(s)} { }
        string_view(const char* s, std::size_t len) : str{s}, len{len} { }

        std::size_t length() const { return len; }
        std::size_t size() const { return len; }
        const char* begin() const { return str; }
        const char* end() const { return str + len; }
        char operator[](std::size_t i) const { return str[i]; }
        string_view substr(std::size_t offset, std::size_t len = -1) const {
            if (len == -1) len = this->len - offset;
            len = std::min(this->len - offset, len);
            return { str + offset, len };
        }
        std::string to_string() const { return { str, len }; }
        bool empty() const { return !len; }
    private:
        const char* str;
        std::size_t len;
    };

    inline std::ostream& operator<<(std::ostream& stream, string_view str) {
        stream << str.to_string();
        return stream;
    }

    inline bool operator==(string_view a, string_view b) {
        if (a.size() != b.size()) return false;
        return !std::strncmp(a.begin(), b.begin(), a.size());
    }

    inline bool operator!=(string_view a, string_view b) {
        return !(a == b);
    }

    inline bool operator<(string_view a, string_view b) {
        return std::lexicographical_compare(a.begin(), a.end(),
                                            b.begin(), b.end());
    }
}

using meta::string_view;

#endif
