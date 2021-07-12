// Provide an uniform interface for basic math functions between AVR libc and newer
// standards that support <cmath>
#pragma once

#ifndef __AVR__
#include <cmath>
#else

// AVR libc doesn't support cmath
#include <math.h>

// Use builtin functions for min/max/abs
template <typename T>
static inline const T min(T a, T b) {
    return __builtin_min((a, b));
}

template <typename T>
static inline const T max(T a, T b) {
    return __builtin_max((a, b));
}

template <typename T>
static inline const T abs(T n) {
    return __builtin_abs((n));
}

#endif
