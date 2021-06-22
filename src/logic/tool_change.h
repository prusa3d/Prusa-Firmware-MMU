#pragma once
#include <stdint.h>
#include "command_base.h"
#include "unload_filament.h"
#include "load_filament.h"

namespace logic {

/// A high-level command state machine
/// Handles the complex logic of tool change
class ToolChange : public CommandBase {
public:
    inline ToolChange()
        : CommandBase() {}

    /// Restart the automaton
    void Reset(uint8_t param) override;

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() override;

    ProgressCode State() const override;

    ErrorCode Error() const override;

private:
    UnloadFilament unl; ///< a high-level command/operation may be used as a building block of other operations as well
    LoadFilament load;
    uint8_t plannedSlot;
};

extern ToolChange toolChange;

} // namespace logic
