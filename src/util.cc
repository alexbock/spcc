#include "util.hh"

bool starts_with(string_view haystack, string_view needle) {
    if (needle.size() > haystack.size()) return false;
    return !haystack.compare(0, needle.size(), needle);
}


bool ends_with(string_view haystack, string_view needle) {
    if (needle.size() > haystack.size()) return false;
    return !haystack.compare(haystack.size() - needle.size(),
                             needle.size(),
                             needle);
}
