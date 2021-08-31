#include "movable_base.h"
#include "motion.h"

namespace modules {
namespace motion {

MovableBase::OperationResult MovableBase::InitMovement(config::Axis axis) {
    if (motion.InitAxis(axis)) {
        PrepareMoveToPlannedSlot();
        state = Moving;
        return OperationResult::Accepted;
    } else {
        state = Failed;
        return OperationResult::Failed;
    }
}

void MovableBase::PerformMove(config::Axis axis) {
    if (!mm::motion.DriverForAxis(axis).GetErrorFlags().Good()) { // @@TODO check occasionally, i.e. not every time?
        // TMC2130 entered some error state, the planned move couldn't have been finished - result of operation is Failed
        tmcErrorFlags = mm::motion.DriverForAxis(axis).GetErrorFlags(); // save the failed state
        state = Failed;
    } else if (mm::motion.QueueEmpty(axis)) {
        // move finished
        state = Ready;
    }
}

} // namespace motion
} // namespace modules
