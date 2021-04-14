#pragma once

namespace hal {
namespace EEPROM {

    /// EEPROM interface
    void WriteByte(uint16_t addr, uint8_t value);
    void UpdateByte(uint16_t addr, uint8_t value);
    uint8_t ReadByte(uint16_t addr);

} // namespace EEPROM
} // namespace hal
