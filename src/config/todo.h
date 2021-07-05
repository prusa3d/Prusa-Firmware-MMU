#pragma once

#ifndef __AVR__
    #define F_CPU 16000000
#endif

// Max step frequency 40KHz
#define MAX_STEP_FREQUENCY 40000

// Minimum stepper rate 120Hz.
#define MINIMAL_STEP_RATE 120

// Step frequency divider (influences the speed tables!)
#define STEP_TIMER_DIVIDER 8
