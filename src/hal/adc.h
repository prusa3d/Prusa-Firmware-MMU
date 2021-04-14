#pragma once

/// Hardware Abstraction Layer for the ADC's

namespace hal {
namespace ADC {

    /// ADC access routines
    uint16_t ReadADC(uint8_t adc);

} // namespace ADC
} // namespace hal
