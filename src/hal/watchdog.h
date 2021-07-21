#pragma once

namespace hal {

/// Hardware Abstraction Layer for the CPU's internal watchdog
namespace watchdog {

/// watchdog interface
void ConfigureWatchDog(uint16_t period);
void ResetWatchDog();

} // namespace watchdog
} // namespace hal
