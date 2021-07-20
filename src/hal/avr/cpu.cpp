#include "../cpu.h"
#include <avr/interrupt.h>
#include "../watchdog.h"

namespace hal {
namespace cpu {

void Init() {
}

void Reset() {
    cli();
    watchdog::Enable(0); //minimum amount of watchdog
    for (;;)
        ; //endless loop while waiting for the watchdog to reset
}

} // namespace CPU
} // namespace hal
