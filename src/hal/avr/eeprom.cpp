#include "../eeprom.h"

namespace hal {
namespace eeprom {

EEPROM eeprom;

uint8_t EEPROM::ReadByte(EEPROM::addr_t /*addr*/) {
    return 0;
}

void EEPROM::UpdateByte(EEPROM::addr_t /*addr*/, uint8_t /*value*/) {
}

} // namespace eeprom
} // namespace hal
