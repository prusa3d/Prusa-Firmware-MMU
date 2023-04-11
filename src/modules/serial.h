/// @file
#pragma once
#include <stdint.h>

namespace modules {

/// module-level API of the serial port.
namespace serial {

bool WriteToUSART(const uint8_t *src, uint8_t len);

bool Available();

uint8_t ConsumeByte();

} // namespace serial

} // namespace modules
