#pragma once
#include <stdint.h>

/// Enable DEBUG_LOGIC to compile debugging and error messages (beware of code base size ;) ) for the logic layer
#define DEBUG_LOGIC

/// Enable DEBUG_LOGIC to compile debugging and error messages (beware of code base size ;) ) for the logic layer
#define DEBUG_MODULES

/// Enable DEBUG_HAL to compile debugging and error messages (beware of code base size ;) ) for the logic layer
#define DEBUG_HAL

/// Debugging macros and tools
namespace debug {

#ifdef DEBUG_LOGIC
extern const char logic[];
#define dbg_logic(x) debug::dbg_usart(debug::logic, x)
#define dbg_logic_P(x) debug::dbg_usart_P(debug::logic, x)
#else
#define dbg_logic(x) /* */
#define dbg_logic_P(x) /* */
#endif

#ifdef DEBUG_MODULES
extern const char modules[];
#define dbg_modules(x) debug::dbg_usart(debug::modules, x)
#define dbg_modules_P(x) debug::dbg_usart_P(debug::modules, x)
#else
#define dbg_modules(x) /* */
#define dbg_modules_P(x) /* */
#endif

#ifdef DEBUG_HAL
extern const char hal[];
#define dbg_hal(x) debug::dbg_usart(debug::hal, x)
#define dbg_hal_P(x) debug::dbg_usart_P(debug::hal, x)
#else
#define dbg_hal(x) /* */
#define dbg_hal_P(x) /* */
#endif

#if defined(DEBUG_LOGIC) || defined(DEBUG_MODULES) || defined(DEBUG_HAL)
/// Dump an error message onto the USART
/// @param layer PROGMEM string
/// @param s RAM string to be printed
void dbg_usart(const char *layer_P, const char *s);

/// Dump an error message onto the USART
/// @param layer PROGMEM string
/// @param s PROGMEM string to be printed
void dbg_usart_P(const char *layer_P, const char *s_P);
#endif

} // namespace debug
