#pragma once
#include <stdint.h>

/// Hardware Abstraction Layer for the ADC's

namespace hal {
namespace adc {

/// ADC access routines
uint16_t ReadADC(uint8_t adc);

} // namespace adc
} // namespace hal
