#ifndef SPCC_SYSTEM_HH
#define SPCC_SYSTEM_HH

#if defined(_WIN32)
#define PLATFORM_WIN32
#elif defined(__unix__) || defined(__APPLE__)
#define PLATFORM_POSIX
#else
#error unknown platform
#endif

#include <cstdio>

namespace platform {
    namespace stream {
        enum class color {
            red,
            green,
            blue,
            yellow,
            magenta,
            white,
        };

        enum class style {
            bold,
        };

        void set_color(FILE*, color);
        void set_style(FILE*, style);
        void reset_attributes(FILE*);
        bool is_terminal(FILE*);
    }
}

#endif
