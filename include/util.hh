#ifndef SPCC_UTIL_HH
#define SPCC_UTIL_HH

#include "string_view.hh"

#include <cstring>

namespace util {
    inline bool ends_with(string_view haystack, string_view needle) {
        if (needle.size() > haystack.size()) return false;
        return haystack.substr(haystack.size() - needle.size()) == needle;
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
}

#endif
