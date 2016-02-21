#pragma once

#include <string>
#include <cstddef>
#include <experimental/string_view>

using std::experimental::string_view;

bool starts_with(string_view haystack, string_view needle);
bool ends_with(string_view haystack, string_view needle);
inline std::size_t maybe_reverse_index(std::size_t i,
                                       std::size_t size,
                                       bool reverse) {
    return reverse ? size - 1 - i : i;
}
