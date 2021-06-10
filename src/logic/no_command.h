#pragma once
#include <stdint.h>
#include "command_base.h"
#include "unload_to_finda.h"

namespace logic {

/// A dummy No-command operation just to make the init of the firmware consistent (and cleaner code during processing)
class NoCommand : public CommandBase {
public:
    inline NoCommand()
        : CommandBase() {}

    /// Restart the automaton
    void Reset(uint8_t /*param*/) override {}

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() override { return true; }
};

extern NoCommand noCommand;

} // namespace logic
