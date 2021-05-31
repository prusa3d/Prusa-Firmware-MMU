#include "finda.h"
#include "../hal/adc.h"

namespace modules {
namespace finda {

FINDA finda;

uint8_t FINDA::Status() const {
    // we can read ADC directly
    return hal::adc::ReadADC(1) > 512;
}

void FINDA::Step() {
    // in this implementation FINDA doesn't need any stepping
}

} // namespace finda
} // namespace modules
