#include "platform.hh"
#include "options.hh"

#include <print>

#if defined(PLATFORM_WIN32)
#include <io.h>
#elif defined(PLATFORM_POSIX)
#include <unistd.h>
#endif

void platform::stream::set_color(FILE* f, color c) {
    if (!options::state.use_color) return;
    if (!is_terminal(f)) return;
#if defined(PLATFORM_WIN32)
    // TODO
#elif defined(PLATFORM_POSIX)
    switch (c) {
        case color::red:
            std::print(f, "\x1B[31m");
            break;
        case color::green:
            std::print(f, "\x1B[32m");
            break;
        case color::blue:
            std::print(f, "\x1B[34m");
            break;
        case color::yellow:
            std::print(f, "\x1B[33m");
            break;
        case color::magenta:
            std::print(f, "\x1B[35m");
            break;
        case color::white:
            std::print(f, "\x1B[97m");
            break;
    }
#endif
}

void platform::stream::set_style(FILE* f, style s) {
    if (!options::state.use_color) return;
    if (!is_terminal(f)) return;
#if defined(PLATFORM_WIN32)
    // no styles on Windows
#elif defined(PLATFORM_POSIX)
    switch (s) {
        case style::bold:
            std::print(f, "\x1B[1m");
            break;
    }
#endif
}

void platform::stream::reset_attributes(FILE* f) {
    std::print(f, "\x1B[0m");
}

bool platform::stream::is_terminal(FILE* f) {
#if defined(PLATFORM_WIN32)
    return _isatty(_fileno(f));
#elif defined(PLATFORM_POSIX)
    return isatty(fileno(f));
#endif
}
