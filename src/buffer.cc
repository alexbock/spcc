#include "buffer.hh"
#include "translator.hh"

#include <cassert>

buffer::buffer(std::string name, std::string data) :
name{std::move(name)},
data{std::move(data)} {
}

buffer::buffer(buffer&&) = default;
buffer::~buffer() = default;

std::string buffer::get_line(std::size_t lno) {
    std::size_t offset = 0, line = 0;
    while (line < lno) {
        assert(offset < data.size());
        if (data[offset] == '\n') ++line;
        ++offset;
    }
    auto begin = offset;
    while (offset < data.size() && data[offset] != '\n') {
        ++offset;
    }
    return data.substr(begin, offset - begin);
}

buffer* buffer::original() {
    if (src) return src->src->original();
    else return this;
}
