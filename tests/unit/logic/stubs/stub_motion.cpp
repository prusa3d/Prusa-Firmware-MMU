#include "motion.h"
#include "stub_motion.h"

namespace modules {
namespace motion {

Motion motion;
AxisSim axes[3] = {
    { 0, 0, false, false, false },
    { 0, 0, false, false, false },
    { 0, 0, false, false, false },
};

void Motion::InitAxis(Axis axis) {
    axes[axis].enabled = true;
}

void Motion::DisableAxis(Axis axis) {
    axes[axis].enabled = false;
}

bool Motion::StallGuard(Axis axis) {
    return axes[axis].stallGuard;
}

void Motion::ClearStallGuardFlag(Axis axis) {
    axes[axis].stallGuard = false;
}

void Motion::PlanMove(int16_t pulley, int16_t idler, int16_t selector, uint16_t feedrate, uint16_t starting_speed, uint16_t ending_speed) {
    axes[Pulley].targetPos = axes[Pulley].pos + pulley;
    axes[Idler].targetPos = axes[Idler].pos + idler;
    axes[Selector].targetPos = axes[Selector].pos + selector;
    // speeds and feedrates are not simulated yet
}

void Motion::PlanMove(Axis axis, int16_t delta, uint16_t feedrate) {
    axes[axis].targetPos = axes[axis].pos + delta;
}

void Motion::Home(Axis axis, bool direction) {
    axes[Pulley].homed = true;
}

void Motion::SetMode(MotorMode mode) {
}

void Motion::Step() {
    for (uint8_t i = 0; i < 3; ++i) {
        if (axes[i].pos != axes[i].targetPos) {
            int8_t dirInc = (axes[i].pos < axes[i].targetPos) ? 1 : -1;
            axes[i].pos += dirInc;
        }
    }
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

/// probably higher-level operations knowing the semantic meaning of axes

} // namespace motion
} // namespace modules
