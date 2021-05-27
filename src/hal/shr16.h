#pragma once

#include <stdint.h>

namespace hal {
namespace shr16 {

/// 16bit shift register (2x74595) interface
///
/// The pinout is hard coded as follows:
/// SHR16_CLK: signal d13 - PC7
/// SHR16_LAT: signal d10 - PB6
/// SHR16_DAT: signal d9  - PB5
///
/// Shift register outputs:
/// LEDS - hardcoded
/// SHR16_LEDG0 = 0x0100
/// SHR16_LEDR0 = 0x0200
/// SHR16_LEDG1 = 0x0400
/// SHR16_LEDR1 = 0x0800
/// SHR16_LEDG2 = 0x1000
/// SHR16_LEDR2 = 0x2000
/// SHR16_LEDG3 = 0x4000
/// SHR16_LEDR3 = 0x8000
/// SHR16_LEDG4 = 0x0040
/// SHR16_LEDR4 = 0x0080
/// SHR16_LED_MSK = 0xffc0
///
/// TMC2130 Direction/Enable signals - hardcoded
/// SHR16_DIR_0 = 0x0001
/// SHR16_ENA_0 = 0x0002
/// SHR16_DIR_1 = 0x0004
/// SHR16_ENA_1 = 0x0008
/// SHR16_DIR_2 = 0x0010
/// SHR16_ENA_2 = 0x0020
///
/// SHR16_DIR_MSK = (SHR16_DIR_0 + SHR16_DIR_1 + SHR16_DIR_2)
/// SHR16_ENA_MSK = (SHR16_ENA_0 + SHR16_ENA_1 + SHR16_ENA_2)
class SHR16 {

public:
    void Init();
    void SetLED(uint16_t led);
    void SetTMCEnabled(uint8_t ena);
    void SetTMCDir(uint8_t dir);

private:
    uint16_t shr16_v;
    void Write(uint16_t v);
};

extern SHR16 shr16;

} // namespace shr16
} // namespace hal
