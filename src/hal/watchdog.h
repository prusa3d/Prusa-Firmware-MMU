#pragma once

/// Hardware Abstraction Layer for the CPU's features and peripherals

namespace hal {
namespace watchdog {

/// watchdog interface
void ConfigureWatchDog(uint16_t period);
void ResetWatchDog();

} // namespace watchdog
} // namespace hal
