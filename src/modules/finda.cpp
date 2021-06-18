#include "finda.h"
#include "../hal/adc.h"
#include "timebase.h"

namespace modules {
namespace finda {

FINDA finda;

void FINDA::Step() {
    debounce::Debouncer::Step(modules::time::timebase.Millis(), hal::adc::ReadADC(1) > adcDecisionLevel);
}

} // namespace finda
} // namespace modules
