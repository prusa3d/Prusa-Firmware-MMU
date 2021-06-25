#pragma once
#include <stdint.h>
#include "debouncer.h"

namespace modules {

/// The finda namespace provides all necessary facilities related to the logical model of the FINDA device the MMU unit.
namespace finda {

/// A model of the FINDA - basically acts as a button with pre-set debouncing
class FINDA : protected debounce::Debouncer {
public:
    /// time interval for debouncing @@TODO specify units
    constexpr static const uint16_t debounce = 100;
    /// ADC decision level when a FINDA is considered pressed/not pressed
    constexpr static const uint16_t adcDecisionLevel = 512;

    inline constexpr FINDA()
        : debounce::Debouncer(debounce) {};

    /// Performs one step of the state machine - reads the ADC, processes debouncing, updates states of FINDA
    void Step();

    using debounce::Debouncer::Pressed;
};

/// The one and only instance of FINDA in the FW
extern FINDA finda;

} // namespace finda
} // namespace modules
