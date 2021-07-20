#pragma once
#include <stdint.h>

namespace hal {

/// Hardware Abstraction Layer for the CPU's internal watchdog
namespace watchdog {

/// watchdog interface
void Enable(uint16_t period);
void Reset();

} // namespace watchdog
} // namespace hal
