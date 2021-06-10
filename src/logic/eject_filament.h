#pragma once
#include <stdint.h>
#include "command_base.h"
#include "unload_filament.h"

namespace logic {

/// A high-level command state machine
/// Handles the complex logic of ejecting filament:
///
/// - Move selector sideways and push filament forward a little bit, so that the user can catch it
/// - Unpark idler at the end so that the user can pull filament out.
/// - If there is still some filament detected by PINDA unload it first.
/// - If we want to eject fil 0-2, move selector to position 4 (right).
/// - If we want to eject fil 3-4, move selector to position 0 (left)
/// Optionally, we can also move the selector to its service position in the future.
/// @param filament filament 0 to 4

class EjectFilament : public CommandBase {
public:
    inline EjectFilament()
        : CommandBase() {}

    /// Restart the automaton
    void Reset(uint8_t param) override;

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() override;

private:
    UnloadFilament unl; ///< a high-level command/operation may be used as a building block of other operations as well
    uint8_t slot;
    void MoveSelectorAside();
};

extern EjectFilament ejectFilament;

} // namespace logic
