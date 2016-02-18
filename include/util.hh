#pragma once

#include <string>
#include <cstddef>

bool starts_with(const std::string& haystack, const std::string& needle);
bool ends_with(const std::string& haystack, const std::string& needle);
inline std::size_t maybe_reverse_index(std::size_t i,
                                       std::size_t size,
                                       bool reverse) {
    return reverse ? size - 1 - i : i;
}
