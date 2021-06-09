#pragma once

#include <stdint.h>
#include "../hal/adc.h"
#include "debouncer.h"

/// Buttons are built on top of the raw ADC API
/// This layer should contain debouncing of buttons and their logical interpretation

namespace modules {
namespace buttons {

struct Button : public debounce::Debouncer {
    inline constexpr Button()
        : debounce::Debouncer(debounce) {}

private:
    /// time interval for debouncing @@TODO specify units
    constexpr static const uint16_t debounce = 100;
};

enum {
    Left = 0,
    Middle,
    Right
};

class Buttons {
    constexpr static const uint8_t N = 3; ///< number of buttons currently supported
    constexpr static const uint8_t adc = 1; ///< ADC index - will be some define or other constant later on
    static uint16_t tmpTiming; ///< subject to removal when we have timers implemented - now used for the unit tests

public:
    inline constexpr Buttons() = default;

    /// State machine step - reads the ADC, processes debouncing, updates states of individual buttons
    void Step(uint16_t rawADC);

    /// @return true if button at index is pressed
    /// @@TODO add range checking if necessary
    inline bool ButtonPressed(uint8_t index) const { return buttons[index].Pressed(); }
    inline bool AnyButtonPressed() const {
        for (uint8_t i = 0; i < N; ++i) {
            if (ButtonPressed(i))
                return true;
        }
        return false;
    }

private:
    Button buttons[N];

    /// Call to the ADC and decode its output into a button index
    /// @returns index of the button pressed or -1 in case no button is pressed
    static int8_t Sample(uint16_t rawADC);
};

extern Buttons buttons;

} // namespace buttons
} // namespace modules
