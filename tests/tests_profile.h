//
// Created by hurin on 11/11/2020.
//

#ifndef CISSON_TESTS_PROFILE_H
#define CISSON_TESTS_PROFILE_H

#include <Windows.h>

LONGLONG start_profiler();
LONGLONG start_timer();
LONGLONG elapsed(LONGLONG start, LONGLONG frequency);

#endif //CISSON_TESTS_PROFILE_H
