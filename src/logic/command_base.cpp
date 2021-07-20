#include "command_base.h"
#include "../modules/idler.h"

namespace mi = modules::idler;

namespace logic {

bool CommandBase::Step() {
    // check the global HW errors - may be we should avoid the modules layer and check for the HAL layer errors directly
    // @@TODO discuss...
    bool any_error = mi::idler.State() == mi::Idler::Failed;

    // @@TODO check all other HW issues here to be able to respond with the appropriate error code into the printer

    if (any_error) {
        state = ProgressCode::ERR1TMCInitFailed;
        error = ErrorCode::TMC_INIT_ERROR;
        return true; // the HW error prevents us from continuing with the with the state machine
        // the MMU must be restarted/fixed before continuing
    } else {
        return StepInner();
    }
}

} // namespace logic
