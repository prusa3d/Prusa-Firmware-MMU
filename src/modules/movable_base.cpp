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

} // namespace motion
} // namespace modules
