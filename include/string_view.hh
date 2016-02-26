#include <string>
#include <cstddef>
#include <cstring>

namespace meta {
    class string_view {
    public:
        string_view(const std::string& s) : str{s.c_str()}, len{s.size()} { }
        string_view(const char* s) : str{s}, len{std::strlen(s)} { }
        string_view(const char* s, std::size_t len) : str{s}, len{len} { }

        std::size_t length() const { return len; }
        std::size_t size() const { return len; }
        const char* begin() const { return str; }
        const char* end() const { return str + len; }
        char operator[](std::size_t i) const { return str[i]; }
        string_view substr(std::size_t offset, std::size_t len) {
            return { str + offset, this->len - len };
        }
        std::string to_string() { return { str, len }; }
    private:
        const char* str;
        std::size_t len;
    };
}

using meta::string_view;
