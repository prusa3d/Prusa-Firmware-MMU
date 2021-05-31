#pragma once

#include <stdint.h>

/// We have 5 pairs of LEDs
/// In each pair there is a green and a red LED
///
/// A typical scenario in the past was visualization of error states.
/// The combination of colors with blinking frequency had a specific meaning.
///
/// The physical connection is not important on this level (i.e. how and what shall be sent into the shift registers)

namespace modules {
namespace leds {

/// Mode of LED
/// blink0 and blink1 allow for interlaced blinking of LEDs (one is on and the other off)
enum Mode {
    off,
    on,
    blink0, ///< start blinking at even periods
    blink1 ///< start blinking at odd periods
};

enum Color {
    green = 0,
    red = 1
};

/// a single LED
class LED {
public:
    constexpr inline LED() = default;
    void SetMode(Mode mode);

    /// @returns true if the LED shines
    bool Step(bool oddPeriod);
    inline bool On() const { return state.on; }

private:
    struct State {
        uint8_t on : 1;
        uint8_t mode : 2;
        constexpr inline State()
            : on(0)
            , mode(Mode::off) {}
    };

    State state;
};

/// main LED API
class LEDs {
public:
    constexpr inline LEDs()
        : ms(0) {};

    /// step LED automaton
    void Step(uint16_t delta_ms);

    inline constexpr uint8_t LedPairsCount() const { return ledPairs; }

    inline void SetMode(uint8_t slot, Color color, Mode mode) {
        SetMode(slot * 2 + color, mode);
    }
    inline void SetMode(uint8_t index, Mode mode) {
        leds[index].SetMode(mode);
    }

    inline bool LedOn(uint8_t index) const {
        return leds[index].On();
    }
    inline bool LedOn(uint8_t slot, Color color) const {
        return leds[slot * 2 + color].On();
    }

private:
    constexpr static const uint8_t ledPairs = 5;
    /// pairs of LEDs:
    /// [0] - green LED slot 0
    /// [1] - red LED slot 0
    /// [2] - green LED slot 1
    /// [3] - red LED slot 1
    /// [4] - green LED slot 2
    /// [5] - red LED slot 2
    /// [6] - green LED slot 3
    /// [7] - red LED slot 3
    /// [8] - green LED slot 4
    /// [9] - red LED slot 4
    LED leds[ledPairs * 2];
    uint16_t ms;
};

extern LEDs leds;

} // namespace LEDs
} // namespace modules
