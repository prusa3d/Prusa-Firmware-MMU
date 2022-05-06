#include "serial.h"

namespace modules {
namespace serial {

bool WriteToUSART(const uint8_t *src, uint8_t len) { return false; }

bool Available() { return false; }

uint8_t ConsumeByte() { return 0; }

} // namespace serial
} // namespace modules
