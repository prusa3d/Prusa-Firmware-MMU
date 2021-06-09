#pragma once
#include <stdint.h>
#include "command_base.h"
#include "unload_to_finda.h"

namespace logic {

/// A high-level command state machine
/// Handles the complex logic of ejecting filament
class EjectFilament : public CommandBase {
public:
    inline EjectFilament()
        : CommandBase() { Reset(); }

    /// Restart the automaton
    void Reset() override;

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() override;

private:
};

extern EjectFilament ejectFilament;

} // namespace logic
