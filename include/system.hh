#if defined(_WIN32)
#define PLATFORM_WIN32
#elif defined(__unix__) || defined(__APPLE__)
#define PLATFORM_POSIX
#else
#error unknown platform
#endif

#include <cstdio>

namespace system {
    namespace stream {
        enum class color {
            standard,
            red,
            green,
            blue,
            yellow,
            magenta,
            white,
        };

        enum class style {
            standard,
            bold,
        };

        void set_color(FILE*, color);
        void set_style(FILE*, style);
        bool is_terminal(FILE*);
    }
}
