#pragma once

#include <stdint.h>
#include "../hal/adc.h"

/// Buttons are built on top of the raw ADC API
/// This layer should contain debouncing of buttons and their logical interpretation

namespace modules {

struct Button {
    inline constexpr Button()
        : state(State::Waiting)
        , tmp(false)
        , pressed(false)
        , timeLastChange(0) {}

    /// @returns true if button is currently considered as pressed
    inline bool Pressed() const { return pressed; }

    /// State machine stepping routine
    void Step(uint16_t time, bool press);

private:
    constexpr static const uint16_t debounce = 100; ///< time interval for debouncing @@TODO specify units
    enum class State : uint_fast8_t { Waiting = 0,
        Detected,
        WaitForRelease,
        Update };
    State state;
    bool tmp; ///< temporary state of button before the debouncing state machine finishes
    bool pressed; ///< real state of button after debouncing
    uint16_t timeLastChange;
};

class Buttons {
    constexpr static const uint8_t N = 3; ///< number of buttons currently supported
    constexpr static const uint8_t adc = 1; ///< ADC index - will be some define or other constant later on
    static uint16_t tmpTiming;

public:
    inline constexpr Buttons() = default;

    /// State machine step - reads the ADC, processes debouncing, updates states of individual buttons
    void Step();

    /// @return true if button at index is pressed
    /// @@TODO add range checking if necessary
    inline bool ButtonPressed(uint8_t index) const { return buttons[index].Pressed(); }

private:
    Button buttons[N];
    static int8_t Sample();
};

} // namespace modules
