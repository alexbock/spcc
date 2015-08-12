#pragma once

#include <vector>
#include <string>

struct options {
    bool enable_color = true;
    bool run_internal_tests = false;
    bool input_from_stdin = false;
    bool parse_error = false;
    bool help_requested = false;
    bool version_requested = false;
    std::vector<std::string> files;
};

void parse_options(int argc, char** argv);

extern options program_options;
