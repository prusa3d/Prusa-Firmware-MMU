#pragma once
#include <stdint.h>

namespace logic {

/// @brief Retract filament from FINDA to PTFE
///
/// Continuously pull filament by a fixed length (originally 600 steps) + verify FINDA is switched OFF
struct RetractFromFinda {
    /// internal states of the state machine
    enum {
        EngagingIdler,
        UnloadBackToPTFE,
        OK,
        Failed
    };

    inline RetractFromFinda()
        : state(OK) {}

    /// Restart the automaton
    void Reset();

    /// @returns true if the state machine finished its job, false otherwise
    bool Step();

    /// This method may be used to check the result of the automaton
    /// @returns OK if everything went OK and FINDA triggered
    /// @returns Failed if the FINDA didn't trigger
    inline uint8_t State() const { return state; }

private:
    uint8_t state;
};

} // namespace logic
