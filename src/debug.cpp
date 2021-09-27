#include "debug.h"

#if defined(DEBUG_LOGIC) || defined(DEBUG_MODULES) || defined(DEBUG_HAL)

#include "hal/usart.h"
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

namespace debug {

#ifdef DEBUG_LOGIC
const char logic[] PROGMEM = "log:";
#endif

#ifdef DEBUG_MODULES
const char modules[] PROGMEM = "mod:";
#endif

#ifdef DEBUG_HAL
const char hal[] PROGMEM = "hal:";
#endif

void dbg_usart(const char *layer_P, const char *s) {
    hu::usart1.puts_P(layer_P);
    hu::usart1.puts(s);
}

void dbg_usart_P(const char *layer_P, const char *s_P) {
    hu::usart1.puts_P(layer_P);
    hu::usart1.puts_P(s_P);
}

} // namespace debug

#endif
