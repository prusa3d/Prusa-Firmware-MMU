#include "../shr16.h"
#include "../gpio.h"
#include "../../pins.h"

#define SHR16_LED_MSK 0xffc0
#define SHR16_DIR_MSK 0x0015
#define SHR16_ENA_MSK 0x002A

namespace hal {
namespace shr16 {

SHR16 shr16;

void SHR16::Init() {
    using namespace hal::gpio;
    gpio::Init(GPIO_pin(SHR16_DATA), GPIO_InitTypeDef(Mode::output, Level::low));
    gpio::Init(GPIO_pin(SHR16_LATCH), GPIO_InitTypeDef(Mode::output, Level::high));
    gpio::Init(GPIO_pin(SHR16_CLOCK), GPIO_InitTypeDef(Mode::output, Level::low));
    Write(0);
}

void SHR16::Write(uint16_t v) {
    using namespace hal::gpio;
    WritePin(GPIO_pin(SHR16_LATCH), Level::low);
    for (uint16_t m = 0x8000; m; m >>= 1)
    {
        WritePin(GPIO_pin(SHR16_DATA), (Level)((m & v) != 0));
        WritePin(GPIO_pin(SHR16_CLOCK), Level::high);
        WritePin(GPIO_pin(SHR16_CLOCK), Level::low);
    }
    WritePin(GPIO_pin(SHR16_LATCH), Level::high);
    shr16_v = v;
}

void SHR16::SetLED(uint16_t led) {
    led = ((led & 0x00ff) << 8) | ((led & 0x0300) >> 2);
    Write((shr16_v & ~SHR16_LED_MSK) | led);
}

void SHR16::SetTMCEnabled(uint8_t index, bool ena) {
    //    ena ^= 7;
    //	ena = ((ena & 1) << 1) | ((ena & 2) << 2) | ((ena & 4) << 3); // 0. << 1 == 1., 1. << 2 == 3., 2. << 3 == 5.
    //	Write((shr16_v & ~SHR16_ENA_MSK) | ena);
}

void SHR16::SetTMCDir(uint8_t index, bool dir) {
    //    dir = (dir & 1) | ((dir & 2) << 1) | ((dir & 4) << 2); // 0., 1. << 1 == 2., 2. << 2 == 4.
    //	Write((shr16_v & ~SHR16_DIR_MSK) | dir);
}

} // namespace shr16
} // namespace hal
