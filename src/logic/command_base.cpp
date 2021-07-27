#include "command_base.h"
#include "../modules/idler.h"
#include "../modules/selector.h"

namespace mi = modules::idler;
namespace ms = modules::selector;

namespace logic {

bool CommandBase::Step() {
    // check the global HW errors - may be we should avoid the modules layer and check for the HAL layer errors directly
    if (mi::idler.State() == mi::Idler::Failed) {
        state = ProgressCode::ERRTMCFailed;
        error = ErrorCode::TMC_IOIN_MISMATCH;
        return true; // the HW error prevents us from continuing with the with the state machine - the MMU must be restarted/fixed before continuing
    } else if (ms::selector.State() == ms::Selector::Failed) {
        state = ProgressCode::ERRTMCFailed;
        error = ErrorCode::TMC_IOIN_MISMATCH;
        return true; // the HW error prevents us from continuing with the with the state machine - the MMU must be restarted/fixed before continuing
    }

    return StepInner();
}

} // namespace logic
