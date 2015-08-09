#include "util.hh"

bool starts_with(const std::string& haystack, const std::string& needle) {
    if (needle.size() > haystack.size()) return false;
    return !haystack.compare(0, needle.size(), needle);
}


bool ends_with(const std::string& haystack, const std::string& needle) {
    if (needle.size() > haystack.size()) return false;
    return !haystack.compare(haystack.size() - needle.size(),
                             needle.size(),
                             needle);
}
