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
        : CommandBase() {}

    /// Restart the automaton
    void Reset(uint8_t param) override;

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() override;

private:
    constexpr static const uint8_t maxRetries = 3;
    UnloadToFinda unl;
};

extern UnloadFilament unloadFilament;

} // namespace logic
