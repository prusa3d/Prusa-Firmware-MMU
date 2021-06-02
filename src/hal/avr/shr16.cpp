#include "../shr16.h"
#include "../gpio.h"

namespace hal {
namespace shr16 {

SHR16 shr16;

void SHR16::Init() {
    //    DDRC |= 0x80;
    //	DDRB |= 0x40;
    //	DDRB |= 0x20;
    //	PORTC &= ~0x80;
    //	PORTB &= ~0x40;
    //	PORTB &= ~0x20;
    //	shr16_v = 0;
    //	Write(shr16_v);
    //	Write(shr16_v);
}

void SHR16::Write(uint16_t v) {
    //    PORTB &= ~0x40;
    //	asm("nop");
    //	for (uint16_t m = 0x8000; m; m >>= 1)
    //	{
    //		if (m & v)
    //			PORTB |= 0x20;
    //		else
    //			PORTB &= ~0x20;
    //		PORTC |= 0x80;
    //		asm("nop");
    //		PORTC &= ~0x80;
    //		asm("nop");
    //	}
    //	PORTB |= 0x40;
    //	asm("nop");
    //	shr16_v = v;
}

void SHR16::SetLED(uint16_t led) {
    //    led = ((led & 0x00ff) << 8) | ((led & 0x0300) >> 2);
    //	Write((shr16_v & ~SHR16_LED_MSK) | led);
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
