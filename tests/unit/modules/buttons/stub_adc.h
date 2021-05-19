#pragma once

#include <stdint.h>
#include <vector>

namespace hal {
namespace ADC {

    using TADCData = std::vector<uint16_t>;

    void ReinitADC(TADCData &&d, uint8_t ovsmpl);

} // namespace ADC
} // namespace hal
