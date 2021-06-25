#pragma once
#include <stdint.h>
#include "debouncer.h"

namespace modules {

/// The fsensor namespace provides all necessary facilities related to the logical model of the printer's filamens sensor device.
namespace fsensor {

/// External module - model of printer's filament sensor
/// The debouncer is probably not necessary, but it has all the necessary functionality for modelling of the filament sensor
class FSensor : protected debounce::Debouncer {
public:
    inline constexpr FSensor()
        : debounce::Debouncer(debounce)
        , reportedFSensorState(false) {};

    /// Performs one step of the state machine - processes a change-of-state message if any arrived
    void Step();

    using debounce::Debouncer::Pressed;

    /// Records a change of state of filament sensor when arrived via communication
    void ProcessMessage(bool on);

private:
    /// time interval for debouncing @@TODO specify units
    constexpr static const uint16_t debounce = 10;
    bool reportedFSensorState; ///< reported state that came from the printer via a communication message
};

/// The one and only instance of printer's filament sensor in the FW
extern FSensor fsensor;

} // namespace fsensor
} // namespace modules
