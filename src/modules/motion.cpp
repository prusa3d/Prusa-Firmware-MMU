#include "motion.h"

namespace modules {
namespace motion {

Motion motion;

void Motion::InitAxis(Axis axis) {}

void Motion::DisableAxis(Axis axis) {}

bool Motion::StallGuard(Axis axis) { return false; }

void Motion::ClearStallGuardFlag(Axis axis) {}

void Motion::PlanMoveTo(Axis axis, pos_t pos, steps_t feedrate) {}

pos_t Motion::CurrentPos(Axis axis) const { return axisData[axis].ctrl.Position(); }

void Motion::Home(Axis axis, bool direction) {}

void Motion::SetMode(Axis axis, MotorMode mode) {}

bool Motion::QueueEmpty() const {
    for (uint8_t i = 0; i != NUM_AXIS; ++i)
        if (!axisData[i].ctrl.QueueEmpty())
            return false;
    return true;
}

void Motion::AbortPlannedMoves() {
    for (uint8_t i = 0; i != NUM_AXIS; ++i)
        axisData[i].ctrl.AbortPlannedMoves();
}

void Motion::Step() {}

void ISR() {}

} // namespace motion
} // namespace modules
