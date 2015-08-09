#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <utility>

struct buffer;

class translator {
public:
    translator(buffer& src, buffer& dst) :
    src{&src}, dst{&dst} {
    }

    void propagate(std::size_t);
    void insert(std::string);
    void erase(std::size_t);
    void replace(std::size_t, std::string);
    std::size_t translate_dst_to_src(std::size_t dst_index);
    bool done() const;
    std::string peek(std::size_t len) const;
    char peek() const;

    buffer* src;
    buffer* dst;
    std::size_t src_index = 0;
private:
    struct fragment {
        std::size_t src_start;
        std::size_t src_end;
        std::size_t dst_start;
        std::size_t dst_end;
        bool translate_local_offset;
    };
    std::vector<fragment> fragments;
};
