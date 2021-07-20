#pragma once
#include <stdint.h>

namespace hal {

/// Hardware Abstraction Layer for the CPU's internal watchdog
namespace watchdog {

#if defined(__AVR__)
constexpr uint32_t F_WDT = 128000; //frequency of the watchdog unit in Hz
constexpr uint32_t basePrescaler = 2048; //what prescalerBits==0 actually does.
constexpr uint8_t maxPrescaler = 9; //the maximum value prescalerBits can take
constexpr uint8_t reloadBits = 0; //number of bits in the reload register
#elif defined(__STM32__) //@todo to be changed to the final form
constexpr uint32_t F_WDT = 32000; //frequency of the watchdog unit in Hz
constexpr uint32_t basePrescaler = 4; //what prescalerBits==0 actually does.
constexpr uint8_t maxPrescaler = 7; //the maximum value prescalerBits can take
constexpr uint8_t reloadBits = 12; //number of bits in the reload register
#endif

struct configuration {
    uint8_t prescalerBits;
    uint16_t reload;

public:
    static constexpr configuration compute(float timeout) {
        constexpr float tickPeriod = 1 / (float)F_WDT;
        uint8_t prescalerBits = 0;
        uint16_t reload = 0;
        while (tickPeriod * (basePrescaler << prescalerBits) < timeout) {
            prescalerBits++;
        }
        if (timeout)
            reload = static_cast<uint16_t>(((timeout * (1 << reloadBits)) / (tickPeriod * (basePrescaler << prescalerBits)))) - 1;

        configuration config = { prescalerBits, reload };
        return config;
    }
};

/// watchdog interface
void Enable(const configuration &config);
void Disable();
void Reset();

} // namespace watchdog
} // namespace hal
