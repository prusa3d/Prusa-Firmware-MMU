#pragma once
#include <stdint.h>
#include "command_base.h"
#include "unload_to_finda.h"

namespace logic {

/// A high-level command state machine
/// Handles the complex logic of cutting filament
class CutFilament : public CommandBase {
public:
    inline CutFilament()
        : CommandBase() { Reset(); }

    /// Restart the automaton
    void Reset() override;

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() override;

private:
};

extern CutFilament cutFilament;

} // namespace logic
