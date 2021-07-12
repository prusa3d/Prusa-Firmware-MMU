#include "motion.h"

namespace modules {
namespace motion {

Motion motion;

// TODO: not implemented
void Motion::InitAxis(Axis axis) {}

void Motion::SetEnabled(Axis axis, bool enabled) {
    axisData[axis].drv.SetEnabled(axisParams[axis].params, enabled);
    axisData[axis].enabled = enabled;
}

void Motion::SetMode(Axis axis, MotorMode mode) {
    for (uint8_t i = 0; i != NUM_AXIS; ++i)
        axisData[axis].drv.SetMode(mode);
}

// TODO: not implemented
bool Motion::StallGuard(Axis axis) {
    return false;
}

// TODO: not implemented
void Motion::ClearStallGuardFlag(Axis axis) {
}

// TODO: not implemented
void Motion::Home(Axis axis, bool direction) {
}

void Motion::PlanMoveTo(Axis axis, pos_t pos, steps_t feedrate) {
    if (axisData[axis].ctrl.PlanMoveTo(pos, feedrate)) {
        // move was queued, prepare the axis
        if (!axisData[axis].enabled)
            SetEnabled(axis, true);
    }
}

pos_t Motion::Position(Axis axis) const {
    return axisData[axis].ctrl.Position();
}

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

st_timer_t Motion::Step() {
    return 0;
}

void ISR() {}

} // namespace motion
} // namespace modules
