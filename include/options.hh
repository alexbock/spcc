#ifndef SPCC_OPTIONS_HH
#define SPCC_OPTIONS_HH

#include <vector>
#include <string>

namespace options {
    enum class run_mode {
        normal,
        show_help,
        show_version,
        run_tests,
        dump_config,
        debug_parse_declarator,
        option_parsing_error,
    };

    struct size_info {
        unsigned bits_per_byte = 8;
        unsigned size_bytes = 8;
        unsigned short_bytes = 2;
        unsigned int_bytes = 4;
        unsigned long_bytes = 4;
        unsigned long_long_bytes = 8;
    };

    struct config {
        std::vector<std::string> input_filenames;
        run_mode mode = run_mode::normal;
        bool use_color = true;
        int exit_code = 0;
        size_info sizes;
        bool is_char_signed = true;

        std::string debug_declarator_to_parse;
    };

    extern config state;

    void parse(int argc, char** argv);
    void dump();
}

#endif
