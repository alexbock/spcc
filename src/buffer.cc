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

void location::add_expansion_entry(location loc, std::size_t depth) {
    if (depth > 10) return;
    if (expanded_from) expanded_from->add_expansion_entry(loc, depth + 1);
    else expanded_from = std::make_shared<location>(loc);
}

std::string_view derived_buffer::peek() const {
    return parent()->data().substr(index_);
}

void derived_buffer::propagate(std::size_t len) {
    if (!fragments_.empty() && fragments_.back().propagate) {
        // combine with previous propagate fragment
        fragments_.back().local_range.second += len;
        fragments_.back().parent_range.second += len;
        data_ += std::string(peek().substr(0, len));
        index_ += len;
        return;
    }

    fragments_.push_back({
        { data().size(), data().size() + len },
        { index_, index_ + len },
        true
    });
    data_ += std::string(peek().substr(0, len));
    index_ += len;
}

void derived_buffer::replace(std::size_t len, std::string_view str) {
    fragments_.push_back({
        { data().size(), data().size() + str.size() },
        { index_, index_ + len },
        false
    });
    index_ += len;
    data_ += std::string(str);
}

void derived_buffer::insert(std::string_view data) {
    replace(0, data);
}

void derived_buffer::erase(std::size_t len) {
    replace(len, "");
}

bool derived_buffer::done() const {
    return index_ == parent()->data().size();
}
