#include "leds.h"

namespace modules {
namespace leds {

    void LED::SetMode(Mode mode) {
        state.mode = mode;
        // set initial state of LEDs correctly - transition from one mode to another
        switch (state.mode) {
        case Mode::blink1:
        case Mode::off:
            state.on = 0;
            break;

        case Mode::blink0:
        case Mode::on:
            state.on = 1;
            break;
        default:
            break;
        }
    }

    bool LED::Step(bool oddPeriod) {
        switch (state.mode) {
        // on and off don't change while stepping
        case Mode::blink0:
            state.on = oddPeriod;
            break;
        case Mode::blink1:
            state.on = !oddPeriod;
            break;
        default: // do nothing
            break;
        }
    }

    uint16_t LEDs::Step(uint8_t delta_ms) {
        ms += delta_ms;
        bool oddPeriod = ((ms / 1000U) & 0x01U) != 0;
        uint16_t result = 0;
        for (uint8_t i = 0; i < ledPairs * 2; ++i) {
            result <<= 1;
            result |= leds[i].Step(oddPeriod);
        }
        return result;
    }

} // namespace leds
} // namespace modules
