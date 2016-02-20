#pragma once

#include "location.hh"

#include <string>
#include <utility>
#include <memory>

struct buffer {
    buffer(std::string name, std::string data);
    buffer(buffer&&);
    ~buffer();

    std::string name;
    std::string data;
    std::unique_ptr<class translator> src;

    buffer* original();
    std::string get_line(std::size_t lno);

    location included_at;
};

using buffer_ptr = std::unique_ptr<buffer>;
