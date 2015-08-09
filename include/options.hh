#pragma once

#include <vector>
#include <string>

struct options {
    bool enable_color = true;
    bool run_internal_tests = false;
    bool input_from_stdin = false;
    bool parse_error = false;
    std::vector<std::string> files;
};

options parse_options(int argc, char** argv);

extern options program_options;
