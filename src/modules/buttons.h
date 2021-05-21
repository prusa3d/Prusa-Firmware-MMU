#pragma once

#include <stdint.h>
#include "../hal/adc.h"

/// Buttons are built on top of the raw ADC API
/// This layer should contain debouncing of buttons and their logical interpretation

namespace modules {
namespace buttons {

    struct Button {
        inline constexpr Button()
            : timeLastChange(0) {}

        /// @returns true if button is currently considered as pressed
        inline bool Pressed() const { return f.state == State::WaitForRelease; }

        /// State machine stepping routine
        void Step(uint16_t time, bool press);

    private:
        /// time interval for debouncing @@TODO specify units
        constexpr static const uint16_t debounce = 100;

        /// States of the debouncing automaton
        /// Intentionally not modeled as an enum class
        /// as it would impose additional casts which do not play well with the struct Flags
        /// and would make the code less readable
        enum State { Waiting = 0,
            Detected,
            WaitForRelease,
            Update };

        /// The sole purpose of this data struct is to save RAM by compressing several flags into one byte on the AVR
        struct Flags {
            uint8_t state : 2; ///< state of the button
            uint8_t tmp : 1; ///< temporary state of button before the debouncing state machine finishes
            inline constexpr Flags()
                : state(State::Waiting)
                , tmp(false) {}
        };

        /// Flags and state of the debouncing automaton
        Flags f;

        /// Timestamp of the last change of ADC state for this button
        uint16_t timeLastChange;
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

    private:
        Button buttons[N];

        /// Call to the ADC and decode its output into a button index
        /// @returns index of the button pressed or -1 in case no button is pressed
        static int8_t Sample(uint16_t rawADC);
    };

} // namespace buttons
} // namespace modules
