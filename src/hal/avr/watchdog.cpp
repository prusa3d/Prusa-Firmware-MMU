#include "../watchdog.h"

namespace hal {
namespace watchdog {

void Enable(uint16_t period) {
    // @@TODO
}

void Reset() {
    asm("wdr");
}

} // namespace watchdog
} // namespace hal
