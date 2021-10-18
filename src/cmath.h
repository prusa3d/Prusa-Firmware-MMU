/// @file cmath.h
// Provide an uniform interface for basic math functions between AVR libc and newer
// standards that support <cmath>
#pragma once

#ifndef __AVR__
#include <cmath> // abs

#include <algorithm>
using std::max;
using std::min;
#else

// AVR libc doesn't support cmath
#include <math.h>

template <typename T>
static inline const T min(T a, T b) {
    return a <= b ? a : b;
}

template <typename T>
static inline const T max(T a, T b) {
    return a > b ? a : b;
}

template <typename T>
static inline const T abs(T n) {
    // Use builtin function when available
    return __builtin_abs((n));
}

#endif
