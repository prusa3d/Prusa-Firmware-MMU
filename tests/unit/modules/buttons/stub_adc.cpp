#include "adc.h"
#include "stub_adc.h"
#include <vector>

namespace hal {
namespace adc {

static TADCData values2Return;
static TADCData::const_iterator rdptr = values2Return.cbegin();
static uint8_t oversampleFactor = 1;
static uint8_t oversample = 1; ///< current count of oversampled values returned from the ADC - will get filled with oversampleFactor once it reaches zero

void ReinitADC(TADCData &&d, uint8_t ovsmpl) {
    values2Return = std::move(d);
    oversampleFactor = ovsmpl;
    oversample = ovsmpl;
    rdptr = values2Return.cbegin();
}

/// ADC access routines
uint16_t ReadADC(uint8_t /*adc*/) {
    if (!oversample) {
        ++rdptr;
        oversample = oversampleFactor;
    } else {
        --oversample;
    }
    return rdptr != values2Return.end() ? *rdptr : 1023;
}

} // namespace adc
} // namespace hal
