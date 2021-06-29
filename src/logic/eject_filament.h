#pragma once
#include <stdint.h>
#include "command_base.h"
#include "unload_filament.h"

namespace logic {

/// @brief  A high-level command state machine - handles the complex logic of ejecting filament
///
/// The eject operation consists of:
/// - Move selector sideways and push filament forward a little bit, so that the user can catch it
/// - Unpark idler at the end so that the user can pull filament out.
/// - If there is still some filament detected by PINDA unload it first.
/// - If we want to eject fil 0-2, move selector to position 4 (right).
/// - If we want to eject fil 3-4, move selector to position 0 (left)
/// Optionally, we can also move the selector to its service position in the future.
class EjectFilament : public CommandBase {
public:
    inline EjectFilament()
        : CommandBase() {}

    /// Restart the automaton
    /// @param param index of filament slot to eject
    void Reset(uint8_t param) override;

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() override;

    ProgressCode State() const override;

    ErrorCode Error() const override;

private:
    constexpr static const uint16_t ejectSteps = 500; //@@TODO
    UnloadFilament unl; ///< a high-level command/operation may be used as a building block of other operations as well
    uint8_t slot;
    void MoveSelectorAside();
};

/// The one and only instance of EjectFilament state machine in the FW
extern EjectFilament ejectFilament;

} // namespace logic
