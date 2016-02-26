#ifndef SPCC_OPTIONS_HH
#define SPCC_OPTIONS_HH

#include <vector>
#include <string>

namespace options {
    struct config {
        std::vector<std::string> input_filenames;
        bool show_help = false;
        bool show_version = false;
        bool use_color = true;
        int exit_code = 0;
    };

    extern config state;

    void parse(int argc, char** argv);
}

#endif
