#include "adc.h"
#include "stub_adc.h"
#include <vector>

namespace hal {
namespace ADC {

    static TADCData values2Return;
    static TADCData::const_iterator rdptr = values2Return.cbegin();

    void ReinitADC(TADCData d) {
        values2Return = d;
        rdptr = values2Return.cbegin();
    }

    /// ADC access routines
    uint16_t ReadADC(uint8_t /*adc*/) { return rdptr != values2Return.end() ? *rdptr++ : 0; }

} // namespace ADC
} // namespace hal
