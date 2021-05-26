#pragma once
#include <stdint.h>

namespace hal {
namespace EEPROM {

    /// EEPROM interface
    void WriteByte(const uint8_t *addr, uint8_t value);
    void UpdateByte(const uint8_t *addr, uint8_t value);
    uint8_t ReadByte(const uint8_t *addr);

    void WriteWord(const uint8_t *addr, uint16_t value);
    void UpdateWord(const uint8_t *addr, uint16_t value);
    uint16_t ReadWord(const uint8_t *addr);

    /// @returns physical end address of EEPROM memory end
    constexpr const uint16_t End();

} // namespace EEPROM
} // namespace hal
