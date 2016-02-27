#include "buffer.hh"

#include <algorithm>

location location::find_spelling_loc() const {
    if (buffer().parent()) {
        location spelling = {
            *buffer().parent(),
            buffer().offset_in_original(offset())
        };
        return spelling.find_spelling_loc();
    } else return *this;
}

static bool operator<(std::size_t i, const derived_buffer::fragment& rhs) {
    return i < rhs.local_range.second;
}

std::size_t derived_buffer::offset_in_original(std::size_t offset) const {
    if (fragments_.empty() && !offset) return 0;
    auto it = std::upper_bound(fragments_.begin(), fragments_.end(), offset);
    if (it != fragments_.end()) {
        auto result = it->parent_range.first;
        if (it->propagate) result += offset - it->local_range.first;
        return result;
    } else {
        return fragments_.back().parent_range.second;
    }
}

string_view derived_buffer::peek() const {
    return parent()->data().substr(index_);
}

void derived_buffer::propagate(std::size_t len) {
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
        true
    });
    data_ += peek().substr(0, len).to_string();
    index_ += len;
}

void derived_buffer::replace(std::size_t len, string_view str) {
    fragments_.push_back({
        { data().size(), data().size() + str.size() },
        { index_, index_ + len },
        false
    });
    index_ += len;
    data_ += str.to_string();
}

void derived_buffer::insert(string_view data) {
    replace(0, data);
}

void derived_buffer::erase(std::size_t len) {
    replace(len, "");
}

bool derived_buffer::done() const {
    return index_ == parent()->data().size();
}
