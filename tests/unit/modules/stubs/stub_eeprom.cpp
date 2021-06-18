#include "catch2/catch.hpp"

#include "../../../../src/hal/eeprom.h"

#include <array>
#include <stddef.h>
#include <stdint.h>

namespace hal {
namespace eeprom {

EEPROM eeprom;

constexpr uint16_t eepromSize = 2048U;
static std::array<uint8_t, eepromSize> EE;

/// EEPROM interface
void EEPROM::WriteByte(EEPROM::addr_t offset, uint8_t value) {
    REQUIRE(offset < EE.size());
    EE[offset] = value;
}

void EEPROM::UpdateByte(EEPROM::addr_t offset, uint8_t value) {
    WriteByte(offset, value);
}

uint8_t EEPROM::ReadByte(EEPROM::addr_t offset) {
    REQUIRE(offset < EE.size());
    return EE[offset];
}

void EEPROM::WriteWord(EEPROM::addr_t offset, uint16_t value) {
    REQUIRE(offset < EE.size() - 1);
    *reinterpret_cast<uint16_t *>(&EE[offset]) = value;
}

void EEPROM::UpdateWord(EEPROM::addr_t offset, uint16_t value) {
    WriteWord(offset, value);
}

uint16_t EEPROM::ReadWord(EEPROM::addr_t offset) {
    REQUIRE(offset < EE.size() - 1);
    return *reinterpret_cast<uint16_t *>(&EE[offset]);
}

} // namespace eeprom
} // namespace hal
