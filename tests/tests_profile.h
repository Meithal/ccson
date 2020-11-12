//
// Created by hurin on 11/11/2020.
//

#ifndef CISSON_TESTS_PROFILE_H
#define CISSON_TESTS_PROFILE_H

#if (!(defined(_WIN32) || defined(_WIN64)) \
|| defined(__CYGWIN__) \
|| defined(__MINGW32__) \
|| defined(__MINGW64__))
#define LONGLONG long long
#define NO_MSVC
#else
#include <Windows.h>
#endif

LONGLONG start_profiler();
LONGLONG start_timer();
LONGLONG elapsed(LONGLONG start, LONGLONG frequency);

#endif //CISSON_TESTS_PROFILE_H
