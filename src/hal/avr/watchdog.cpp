#include "../watchdog.h"

namespace hal {
namespace watchdog {

void ConfigureWatchDog(uint16_t period) {
    // @@TODO
}

void ResetWatchDog() {
    asm("wdr");
}

} // namespace watchdog
} // namespace hal
