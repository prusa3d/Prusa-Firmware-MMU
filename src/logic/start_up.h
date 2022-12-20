/// @file no_command.h
#pragma once
#include <stdint.h>
#include "command_base.h"

namespace logic {

/// @brief Firmware start up sequence with error handling & reporting
class StartUp : public CommandBase {
public:
    inline StartUp()
        : CommandBase() {}

    /// Restart the automaton
    bool Reset(uint8_t /*param*/) override { return true; }

    /// @returns true if the state machine finished its job, false otherwise
    bool StepInner() override;

    /// Used to report initialization errors (which can be reported if the UART started up).
    /// Intentionally only available in the "noCommand" operation
    /// which is only active when the MMU starts and before it gets any other command from the printer.
    inline void SetInitError(ErrorCode ec) {
        error = ec;
        state = ProgressCode::ERRWaitingForUser;
    }
};

/// The one and only instance of StartUp state machine in the FW
extern StartUp startUp;

} // namespace logic
