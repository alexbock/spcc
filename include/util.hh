#ifndef SPCC_UTIL_HH
#define SPCC_UTIL_HH

#include "string_view.hh"

#include <cstring>

namespace util {
    bool ends_with(string_view haystack, string_view needle) {
        if (needle.size() > haystack.size()) return false;
        return haystack.substr(haystack.size() - needle.size()) == needle;
    }
}

#endif
