#pragma once
#include <stdint.h>
#include "debouncer.h"

/// External module - model of printer's fsensor

namespace modules {
namespace fsensor {

/// the debouncer is probably not necessary, but it has all the necessary functionality for modelling of the fsensor

class FSensor : protected debounce::Debouncer {
public:
    inline constexpr FSensor()
        : debounce::Debouncer(debounce)
        , reportedFSensorState(false) {};
    void Step(uint16_t time);
    using debounce::Debouncer::Pressed;

    void ProcessMessage(bool on);

private:
    /// time interval for debouncing @@TODO specify units
    constexpr static const uint16_t debounce = 10;
    bool reportedFSensorState; ///< reported state that came from the printer via a communication message
};

extern FSensor fsensor;

} // namespace finda
} // namespace modules
