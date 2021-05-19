#pragma once

#include <stdint.h>
#include <vector>

namespace hal {
namespace ADC {

    using TADCData = std::vector<uint16_t>;

    void ReinitADC(TADCData d);

} // namespace ADC
} // namespace hal
