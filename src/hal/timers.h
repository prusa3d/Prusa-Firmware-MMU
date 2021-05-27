#pragma once

/// Hardware Abstraction Layer for the CPU's features and peripherals

namespace hal {
namespace timers {

/// timers
void ConfigureTimer(uint8_t timer /* some config struct */);
void StartTimer(uint8_t timer);
void StopTimer(uint8_t timer);

} // namespace cpu
} // namespace hal
