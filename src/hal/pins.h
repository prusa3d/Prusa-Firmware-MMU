#pragma once

/// Hardware Abstraction Layer for the CPU's features and peripherals

namespace hal {
namespace pins {

    /// pin access routines
    void WritePin(uint8_t pin, uint8_t value);
    uint8_t ReadPin(uint8_t pin);

} // namespace pins
} // namespace hal
