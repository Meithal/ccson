//
// Created by hurin on 11/11/2020.
//

#include "tests_profile.h"

#ifndef NO_MSVC
LONGLONG
start_profiler() {
    LARGE_INTEGER frequency;

    QueryPerformanceFrequency(&frequency);

    return frequency.QuadPart;
}

LONGLONG
start_timer() {
    LARGE_INTEGER starting_time;
    QueryPerformanceCounter(&starting_time);

    return starting_time.QuadPart;
}

LONGLONG
elapsed(LONGLONG start, LONGLONG frequency) {
    LARGE_INTEGER ending_time;
    QueryPerformanceCounter(&ending_time);
    LARGE_INTEGER elapsed_microseconds = {
            .QuadPart=ending_time.QuadPart - start
    };

//    elapsed_microseconds.QuadPart *= 1000000;
//    elapsed_microseconds.QuadPart /= frequency;

    return elapsed_microseconds.QuadPart;
}
#else
LONGLONG
start_profiler() {
    return 0LL;
}

LONGLONG
start_timer() {
    return 0LL;
}

LONGLONG
elapsed(LONGLONG start, LONGLONG frequency) {
    return 0LL;
}

#endif