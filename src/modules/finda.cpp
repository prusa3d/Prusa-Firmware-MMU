/// @file finda.cpp
#include "finda.h"
#include "timebase.h"
#include "../hal/gpio.h"
#include "../pins.h"

namespace modules {
namespace finda {

FINDA finda;

void FINDA::Step() {

    debounce::Debouncer::Step(mt::timebase.Millis(), hal::gpio::ReadPin(FINDA_PIN) == hal::gpio::Level::high);
}

} // namespace finda
} // namespace modules
