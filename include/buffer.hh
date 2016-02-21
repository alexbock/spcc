#pragma once

#include "location.hh"

#include <string>
#include <utility>
#include <memory>
#include <experimental/string_view>

using std::experimental::string_view;

struct buffer {
    buffer(std::string name, std::string data);
    buffer(buffer&&);
    ~buffer();

    std::string name;
    std::string data;
    std::unique_ptr<class translator> src;

    buffer* original();
    std::string get_line(std::size_t lno);
    string_view view() const { return data; }

    location included_at;
};

using buffer_ptr = std::unique_ptr<buffer>;
