#include "platform.hh"

#if defined(PLATFORM_WIN32)
#include <Windows.h>
#include <io.h>
#include <stdio.h>
#endif

#if defined(PLATFORM_POSIX)
#include <unistd.h>
#include <stdio.h>
#endif

bool is_stdout_a_terminal() {
#if defined(PLATFORM_WIN32)
    return _isatty(_fileno(stdout));
#elif defined(PLATFORM_POSIX)
    return isatty(fileno(stdout));
#endif
}
