/// @file movable_base.cpp
#include "movable_base.h"
#include "globals.h"
#include "motion.h"

namespace modules {
namespace motion {

void MovableBase::PlanHome() {
    InvalidateHoming();

    // switch to normal mode on this axis
    mm::motion.InitAxis(axis);
    mm::motion.SetMode(axis, mm::Normal);
    mm::motion.StallGuardReset(axis);

    // plan move at least as long as the axis can go from one side to the other
    PlanHomingMoveForward();
    state = HomeForward;
    currentSlot = -1; // important - other state machines may be waiting for a valid Slot() which is not yet correct while homing in progress
}

MovableBase::OperationResult MovableBase::InitMovement() {
    if (motion.InitAxis(axis)) {
        mm::motion.StallGuardReset(axis);
        PrepareMoveToPlannedSlot();
        state = Moving;
        return OperationResult::Accepted;
    } else {
        state = TMCFailed;
        return OperationResult::Failed;
    }
}

void MovableBase::PerformMove() {
    if (!mm::motion.DriverForAxis(axis).GetErrorFlags().Good()) { // @@TODO check occasionally, i.e. not every time?
        // TMC2130 entered some error state, the planned move couldn't have been finished - result of operation is Failed
        tmcErrorFlags = mm::motion.DriverForAxis(axis).GetErrorFlags(); // save the failed state
        state = TMCFailed;
    } else if (SupportsHoming() && (!mg::globals.MotorsStealth()) && mm::motion.StallGuard(axis)) {
        // Axis stalled while moving - dangerous especially with the Selector
        // Checked only for axes which support homing (because we plan a homing move after the error is resolved to regain precise position)
        mm::motion.StallGuardReset(axis);
        mm::motion.AbortPlannedMoves(axis, true);
        // @@TODO move a bit back from where it came from to enable easier removal of whatever is blocking the axis
        state = MoveFailed;
    } else if (mm::motion.QueueEmpty(axis)) {
        // move finished
        currentSlot = plannedSlot;
        FinishMove();
        state = Ready;
    }
}

void MovableBase::PerformHomeForward() {
    if (mm::motion.StallGuard(axis)) {
        // we have reached the front end of the axis - first part homed probably ok
        mm::motion.StallGuardReset(axis);
        mm::motion.AbortPlannedMoves(axis, true);
        PlanHomingMoveBack();
        state = HomeBack;
    } else if (mm::motion.QueueEmpty(axis)) {
        HomeFailed();
    }
}

void MovableBase::PerformHomeBack() {
    if (mm::motion.StallGuard(axis)) {
        // we have reached the back end of the axis - second part homed probably ok
        mm::motion.StallGuardReset(axis);
        mm::motion.AbortPlannedMoves(axis, true);
        mm::motion.SetMode(axis, mg::globals.MotorsStealth() ? mm::Stealth : mm::Normal);
        if (!FinishHomingAndPlanMoveToParkPos()) {
            // the measured axis' length was incorrect, something is blocking it, report an error, homing procedure terminated
            HomeFailed();
        } else {
            homingValid = true;
            // state = Ready; // not yet - we have to move to our parking or target position after homing the axis
        }
    } else if (mm::motion.QueueEmpty(axis)) {
        HomeFailed();
    }
}

void MovableBase::HomeFailed() {
    // we ran out of planned moves but no StallGuard event has occurred
    // or the measured length of axis was not within the accepted tolerance
    homingValid = false;
    mm::motion.Disable(axis); // disable power to the axis - allows the user to do something with the device manually
    state = HomingFailed;
}

} // namespace motion
} // namespace modules
