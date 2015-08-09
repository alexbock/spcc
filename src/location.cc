#include "location.hh"
#include "buffer.hh"
#include "translator.hh"

std::pair<std::size_t, std::size_t> location::get_line_col() const {
    std::size_t line = 0, col = 0;
    for (std::size_t i = 0; i < index; ++i) {
        ++col;
        if (buf->data[i] == '\n') {
            col = 0;
            ++line;
        }
    }
    return { line, col };
}

location location::spelling() const {
    if (!buf->src) return *this;
    auto src_index = buf->src->translate_dst_to_src(index);
    return location{*buf->src->src, src_index}.spelling();
}
