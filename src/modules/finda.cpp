#include "finda.h"
#include "../hal/adc.h"

namespace modules {
namespace finda {

FINDA finda;

void FINDA::Step(uint16_t time) {
    debounce::Debouncer::Step(time, hal::adc::ReadADC(1) > adcDecisionLevel);
}

} // namespace finda
} // namespace modules
