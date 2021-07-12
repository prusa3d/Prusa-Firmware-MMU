#pragma once
#include <stdint.h>

/// Hardware Abstraction Layer for the ADC's

namespace hal {
namespace adc {

/// ADC access routines
void Init();
uint16_t ReadADC(uint8_t channel);

} // namespace adc
} // namespace hal
