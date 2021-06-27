#include "timebase.h"
#include "../hal/timers.h"

namespace modules {
namespace time {

Timebase timebase;

void Timebase::Init() {
}

void Timebase::ISR() {
}

uint16_t Timebase::Millis() const {
    return ms;
}

} // namespace time
} // namespace modules
