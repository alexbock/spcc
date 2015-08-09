#pragma once

#if defined(_WIN32)
#define PLATFORM_WIN32
#elif defined(__unix__) || defined(__APPLE__)
#define PLATFORM_POSIX
#else
#error unknown platform
#endif

bool is_stdout_a_terminal();
