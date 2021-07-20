#include "../watchdog.h"
#include <avr/wdt.h>

namespace hal {
namespace watchdog {

void Enable(uint16_t period) {
    // @@TODO
}

void Disable() {
    wdt_disable();
}

void Reset() {
    wdt_reset();
}

} // namespace watchdog
} // namespace hal
