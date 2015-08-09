#include "color.hh"
#include "platform.hh"
#include "options.hh"

#include <iostream>

void set_color(color c) {
    if (!program_options.enable_color) return;
    if (!is_stdout_a_terminal()) return;
#if defined(PLATFORM_WIN32)
    // TODO
#elif defined(PLATFORM_POSIX)
    switch (c) {
        case color::standard:
            std::cout << "\033[0m";
            break;
        case color::red:
            std::cout << "\x1B[31m";
            break;
        case color::green:
            std::cout << "\x1B[32m";
            break;
        case color::yellow:
            std::cout << "\x1B[33m";
            break;
    }
#endif
}
