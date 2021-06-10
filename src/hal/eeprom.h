#pragma once
#include <stdint.h>

namespace hal {
namespace eeprom {

/// EEPROM interface
extern void WriteByte(const uint8_t *addr, uint8_t value);
extern void UpdateByte(const uint8_t *addr, uint8_t value);
extern uint8_t ReadByte(const uint8_t *addr);

extern void WriteWord(const uint8_t *addr, uint16_t value);
extern void UpdateWord(const uint8_t *addr, uint16_t value);
extern uint16_t ReadWord(const uint8_t *addr);

/// @returns physical end address of EEPROM memory end
extern constexpr const uint16_t End();

} // namespace EEPROM
} // namespace hal
