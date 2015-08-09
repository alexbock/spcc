#include "translator.hh"
#include "buffer.hh"

#include <algorithm>
#include <cassert>

void translator::propagate(std::size_t len) {
    // if the previous operation was also propagate, just extend it
    if (!fragments.empty() && fragments.back().translate_local_offset) {
        fragments.back().src_end += len;
        fragments.back().dst_end += len;
        dst->data += src->data.substr(src_index, len);
        src_index += len;
        return;
    }

    fragments.push_back({
        src_index,
        src_index + len,
        dst->data.size(),
        dst->data.size() + len,
        true
    });
    dst->data += src->data.substr(src_index, len);
    src_index += len;
}

void translator::insert(std::string str) {
    replace(0, str);
}

void translator::erase(std::size_t len) {
    replace(len, "");
}

void translator::replace(std::size_t len, std::string str) {
    fragments.push_back({
        src_index,
        src_index + len,
        dst->data.size(),
        dst->data.size() + str.size()
    });
    src_index += len;
    dst->data += str;
}

std::size_t translator::translate_dst_to_src(std::size_t dst_index) {
    if (fragments.empty() && !dst_index) return 0;
    if (dst_index >= dst->data.size()) return src->data.size();
    auto comp = [](std::size_t val, const fragment& elm) {
        return val < elm.dst_end;
    };
    auto it = std::upper_bound(fragments.begin(),
                               fragments.end(),
                               dst_index,
                               comp);
    assert(it != fragments.end());
    auto offset = it->src_start;
    if (it->translate_local_offset) {
        offset += (dst_index - it->dst_start);
    }
    return offset;
}

bool translator::done() const {
    return src_index == src->data.size();
}

std::string translator::peek(std::size_t len) const {
    return src->data.substr(src_index, len);
}

char translator::peek() const {
    return src->data[src_index];
}
