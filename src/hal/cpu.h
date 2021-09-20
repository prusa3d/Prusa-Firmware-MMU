#pragma once

namespace hal {

/// Hardware Abstraction Layer for the CPU
namespace cpu {

#ifndef F_CPU
/// Main clock frequency
#define F_CPU (16000000ul)
#endif

/// CPU init routines (not really necessary for the AVR)
void Init();
void Reset();

} // namespace cpu
} // namespace hal
