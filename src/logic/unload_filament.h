#pragma once
#include <stdint.h>
#include "command_base.h"
#include "unload_to_finda.h"

namespace logic {

/// A high-level command state machine
/// Handles the complex logic of unloading filament
class UnloadFilament : public CommandBase {
public:
    inline UnloadFilament()
        : CommandBase()
        , unl(3) { Reset(); }

    /// Restart the automaton
    void Reset() override;

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() override;

private:
    UnloadToFinda unl;
};

extern UnloadFilament unloadFilament;

} // namespace logic
