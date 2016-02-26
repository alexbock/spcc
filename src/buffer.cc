#include "buffer.hh"

#include <algorithm>

namespace lex {
    static bool operator<(std::size_t i, const derived_buffer::fragment& rhs) {
        return i < rhs.local_range.second;
    }
}

std::size_t lex::derived_buffer::offset_in_original(std::size_t offset) const {
    if (fragments_.empty() && !offset) return 0;
    auto it = std::upper_bound(fragments_.begin(), fragments_.end(), offset);
    assert(it != fragments_.end());
    auto result = it->parent_range.first;
    if (it->propagate) result += offset - it->local_range.first;
    return result;
}

string_view lex::derived_buffer::peek() const {
    return parent()->data().substr(index_);
}

void lex::derived_buffer::propagate(std::size_t len) {
    if (!fragments_.empty() && fragments_.back().propagate) {
        // combine with previous propagate fragment
        fragments_.back().local_range.second += len;
        fragments_.back().parent_range.second += len;
        data_ += peek().substr(0, len).to_string();
        index_ += len;
        return;
    }

    fragments_.push_back({
        { data().size(), data().size() + len },
        { index_, index_ + len },
        false
    });
    index_ += len;
    data_ += peek().substr(0, len).to_string();

}

void lex::derived_buffer::replace(std::size_t len, string_view str) {
    fragments_.push_back({
        { data().size(), data().size() + str.size() },
        { index_, index_ + len },
        false
    });
    index_ += len;
    data_ += str.to_string();
}

void lex::derived_buffer::insert(string_view data) {
    replace(0, data);
}

void lex::derived_buffer::erase(std::size_t len) {
    replace(len, "");
}
