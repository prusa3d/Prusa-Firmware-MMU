#pragma once
#include <stdint.h>

/// Unload to FINDA "small" state machine
/// "small" state machines will serve as building blocks for high-level commands/operations
/// - engage/disengage idler
/// - rotate pulley to some direction as long as the FINDA is on/off
/// - rotate some axis to some fixed direction
/// - load/unload to finda

namespace logic {

/// A "small" automaton example - Try to unload filament to FINDA and if it fails try to recover several times.
/// \dot
/// digraph example {
///    node [shape=record, fontname=Helvetica, fontsize=10];
///    b [ label="class B" URL="\ref B"];
///    c [ label="class C" URL="\ref C"];
///    b -> c [ arrowhead="open", style="dashed" ];
///}
///\enddot
struct UnloadToFinda {
    enum {
        EngagingIdler,
        WaitingForFINDA,
        OK,
        Failed
    };
    inline UnloadToFinda()
        : maxTries(3) {}

    /// Restart the automaton
    void Reset(uint8_t maxTries);

    /// @returns true if the state machine finished its job, false otherwise
    bool Step();

    inline uint8_t State() const { return state; }

private:
    uint8_t state;
    uint8_t maxTries;
};

} // namespace logic
