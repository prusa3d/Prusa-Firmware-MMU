#pragma once

#include <stdint.h>

namespace hal {

/// 16bit shift register (2x74595) interface
///
/// The pinout is hard coded as follows:
///                         32u4 port   Schematics
/// SHR16_CLK: signal       D13 - PC7   U4 -> U2/U9 - SHCP and P6 - Pin2
/// SHR16_LAT: signal       D10 - PB6   U4 -> U2/9  - STCP and P6 - Pin3
/// SHR16_DAT: signal       D9  - PB5   U4 -> U2 - DS
///
/// Shift register outputs:
/// LEDS - hardcoded        74HC595     Schematics
/// SHR16_LEDG4 = 0x0100    Q0          U9 -> D14 clostest LED to usb port J1
/// SHR16_LEDR4 = 0x0200    Q1          U9 -> D15
/// SHR16_LEDG3 = 0x0400    Q2          U9 -> D16
/// SHR16_LEDR3 = 0x0800    Q3          U9 -> D17
/// SHR16_LEDG2 = 0x1000    Q4          U9 -> D18
/// SHR16_LEDR2 = 0x2000    Q5          U9 -> D19
/// SHR16_LEDG1 = 0x4000    Q6          U9 -> D20
/// SHR16_LEDR1 = 0x8000    Q7          U9 -> D21
/// SHR16_LEDG0 = 0x0040    Q6          U2 -> D22
/// SHR16_LEDR0 = 0x0080    Q7          U2 -> D23 closest LED to Button S2
/// SHR16_LED_MSK = 0xffc0
///
/// TMC2130 Direction/Enable signals - hardcoded
///                         74HC595     Schematics
/// SHR16_DIR_0 = 0x0001    Q1          U2 -> U5 - DIR
/// SHR16_ENA_0 = 0x0002    Q2          U2 -> U5 - DRV-ENN
/// SHR16_DIR_1 = 0x0004    Q3          U2 -> U6 - DIR
/// SHR16_ENA_1 = 0x0008    Q4          U2 -> U6 - DRV-ENN
/// SHR16_DIR_2 = 0x0010    Q5          U2 -> U7 - DIR
/// SHR16_ENA_2 = 0x0020    Q6          U2 -> U7 - DRV-ENN
///
/// SHR16_DIR_MSK = (SHR16_DIR_0 + SHR16_DIR_1 + SHR16_DIR_2)
/// SHR16_ENA_MSK = (SHR16_ENA_0 + SHR16_ENA_1 + SHR16_ENA_2)
namespace shr16 {

class SHR16 {

public:
    void Init();
    void SetLED(uint16_t led);

    /// indices: PULLEY = 0, SELECTOR = 1, IDLER = 2
    void SetTMCEnabled(uint8_t index, bool ena);
    void SetTMCDir(uint8_t index, bool dir);

private:
    uint16_t shr16_v;
    void Write(uint16_t v);
};

extern SHR16 shr16;

} // namespace shr16
} // namespace hal
