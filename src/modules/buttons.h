#pragma once

#include "../hal/adc.h"

/// Buttons are built on top of the raw ADC API
/// This layer should contain debouncing of buttons and their logical interpretation

namespace modules {

struct Button {
    /// @returns true if button is currently considered as pressed
    bool Pressed() const;

private:
    uint8_t lastState;
    uint16_t timeElapsedLastChange;
};

class Buttons {
public:
    /// State machine step - reads the ADC, processes debouncing, updates states of individual buttons
    void Step();

    /// @return true if button at index is pressed
    /// @@TODO add range checking if necessary
    inline bool ButtonPressed(uint8_t index) const { return buttons[index].Pressed(); }

private:
    Button buttons[3]; ///< @@TODO parametrize/generalize?
};

} // namespace modules
