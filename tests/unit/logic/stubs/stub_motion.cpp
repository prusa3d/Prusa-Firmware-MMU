#include "motion.h"
#include "stub_motion.h"

namespace modules {
namespace motion {

Motion motion;

// Intentionally inited with strange values
// Need to call ReinitMotion() each time we start some unit test
AxisSim axes[3] = {
    { -32767, -32767, false, false, false }, // pulley
    { -32767, -32767, false, false, false }, // selector //@@TODO proper selector positions once defined
    { -32767, -32767, false, false, false }, // idler
};

void Motion::InitAxis(Axis axis) {
    axes[axis].enabled = true;
}

void Motion::SetEnabled(Axis axis, bool enabled) {
    axes[axis].enabled = enabled;
}

bool Motion::StallGuard(Axis axis) {
    return axes[axis].stallGuard;
}

void Motion::ClearStallGuardFlag(Axis axis) {
    axes[axis].stallGuard = false;
}

void Motion::PlanMoveTo(Axis axis, pos_t pos, steps_t feedrate) {
    axes[axis].targetPos = pos;
}

pos_t Motion::Position(Axis axis) const {
    return axes[axis].pos;
}

void Motion::Home(Axis axis, bool direction) {
    axes[Pulley].homed = true;
}

void Motion::SetMode(Axis axis, hal::tmc2130::MotorMode mode) {
}

st_timer_t Motion::Step() {
    for (uint8_t i = 0; i < 3; ++i) {
        if (axes[i].pos != axes[i].targetPos) {
            int8_t dirInc = (axes[i].pos < axes[i].targetPos) ? 1 : -1;
            axes[i].pos += dirInc;
        }
    }
    return 0;
}

bool Motion::QueueEmpty() const {
    for (uint8_t i = 0; i < 3; ++i) {
        if (axes[i].pos != axes[i].targetPos)
            return false;
    }
    return true;
}

void Motion::AbortPlannedMoves() {
    for (uint8_t i = 0; i < 3; ++i) {
        axes[i].targetPos = axes[i].pos; // leave the axis where it was at the time of abort
    }
}

void ReinitMotion() {
    // reset the simulation data to defaults
    axes[0] = AxisSim({ 0, 0, false, false, false }); // pulley
    axes[1] = AxisSim({ 1, 1, false, false, false }); // selector //@@TODO proper selector positions once defined
    axes[2] = AxisSim({ 0, 0, false, false, false }); // idler
}

/// probably higher-level operations knowing the semantic meaning of axes

} // namespace motion
} // namespace modules
