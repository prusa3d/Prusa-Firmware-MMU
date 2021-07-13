#pragma once

/// Hardware Abstraction Layer for the CPU

namespace hal {
namespace cpu {

#ifndef F_CPU
/// Main clock frequency
#define F_CPU (16000000ul)
#endif

/// CPU init routines (not really necessary for the AVR)
void Init();

} // namespace cpu
} // namespace hal
