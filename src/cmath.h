// Provide an uniform interface for basic math functions between AVR libc and newer
// standards that support <cmath>
#pragma once

#ifndef __AVR__
    #include <cmath>
#else

    // AVR libc doesn't support cmath
    #include <math.h>

    // Use builtin functions for min/max/abs
    #define min(a, b) __builtin_min((a, b))
    #define max(a, b) __builtin_max((a, b))
    #define abs(n) __builtin_abs((n))

#endif
