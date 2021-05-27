#include "shr16.h"

namespace hal {
namespace shr16 {

SHR16 shr16;

uint16_t shr16_v_copy;

void SHR16::Init() {
    shr16_v_copy = 0;
}

void SHR16::SetLED(uint16_t led) {
    shr16_v_copy = ((led & 0x00ff) << 8) | ((led & 0x0300) >> 2);
}
void SHR16::SetTMCEnabled(uint8_t ena) {
    // do nothing right now
}
void SHR16::SetTMCDir(uint8_t dir) {
    // do nothing right now
}
void SHR16::Write(uint16_t v) {
    // do nothing right now
}

} // namespace shr16
} // namespace hal
