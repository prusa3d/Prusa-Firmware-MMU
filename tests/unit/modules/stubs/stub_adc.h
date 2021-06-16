#pragma once

#include <stdint.h>
#include <vector>

namespace hal {
namespace adc {

using TADCData = std::vector<uint16_t>;

void ReinitADC(uint8_t channel, TADCData &&d, uint8_t ovsmpl);

} // namespace adc
} // namespace hal
