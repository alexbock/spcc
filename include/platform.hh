#pragma once

#define PROGRAM_VERSION "0.1"

#if defined(_WIN32)
#define PLATFORM_WIN32
#elif defined(__unix__) || defined(__APPLE__)
#define PLATFORM_POSIX
#else
#error unknown platform
#endif

#if defined(PLATFORM_WIN32)
#define PLATFORM_NAME "Windows"
#elif defined(PLATFORM_POSIX)
#define PLATFORM_NAME "POSIX"
#endif

bool is_stdout_a_terminal();
