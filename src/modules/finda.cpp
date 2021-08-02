#include "finda.h"
#include "../hal/adc.h"
#include "timebase.h"

namespace modules {
namespace finda {

FINDA finda;

void FINDA::Step() {
    debounce::Debouncer::Step(mt::timebase.Millis(), hal::adc::ReadADC(config::findaADCIndex) > config::findaADCDecisionLevel);
}

} // namespace finda
} // namespace modules
