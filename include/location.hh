#pragma once

#include <cstddef>
#include <utility>

struct buffer;

struct location {
    location(buffer& buffer, std::size_t index) :
    buf{&buffer},
    index{index} {
    }

    buffer* buf = nullptr;
    std::size_t index = 0;

    operator bool() const { return this->buf; }
    std::pair<std::size_t, std::size_t> get_line_col() const;
    location spelling() const;
};
