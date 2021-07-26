#include "motion.h"

namespace modules {
namespace motion {

Motion motion;

void Motion::InitAxis(Axis axis) {
    for (uint8_t i = 0; i != NUM_AXIS; ++i) {
        // disable the axis and re-init the driver: this will clear the internal
        // StallGuard data as a result without special handling
        Disable(axis);
        axisData[axis].drv.Init(axisParams[axis].params);
    }
}

void Motion::SetEnabled(Axis axis, bool enabled) {
    if (enabled != axisData[axis].enabled) {
        axisData[axis].drv.SetEnabled(axisParams[axis].params, enabled);
        axisData[axis].enabled = enabled;

        if (!axisData[axis].enabled) {
            // axis is powered off, clear internal StallGuard counters
            axisData[axis].stall_trig = false;
            axisData[axis].stall_cnt = 0;
        }
    } // else skip unnecessary Enable/Disable operations on an axis if already in the desired state
}

void Motion::SetMode(Axis axis, MotorMode mode) {
    for (uint8_t i = 0; i != NUM_AXIS; ++i)
        axisData[axis].drv.SetMode(mode);
}

bool Motion::StallGuard(Axis axis) {
    return axisData[axis].stall_trig;
}

void Motion::ClearStallGuardFlag(Axis axis) {
    axisData[axis].stall_trig = false;
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
    st_timer_t timers[NUM_AXIS];

    // step and calculate interval for each new move
    for (uint8_t i = 0; i != NUM_AXIS; ++i) {
        timers[i] = axisData[i].residual;
        if (timers[i] <= config::stepTimerQuantum) {
            timers[i] += axisData[i].ctrl.Step(axisParams[i].params);

            // axis has been moved, sample StallGuard
            if (hal::tmc2130::TMC2130::Stall(axisParams[i].params)) {
                // TODO: on the MK3 a stall is marked as such as 1/2 of a full step is
                // lost: this is too simplistic for production
                ++axisData[i].stall_cnt;
                axisData[i].stall_trig = true;
            }
        }
    }

    // plan next closest interval
    st_timer_t next = timers[0];
    for (uint8_t i = 1; i != NUM_AXIS; ++i) {
        if (timers[i] && (!next || timers[i] < next))
            next = timers[i];
    }

    // update residuals
    for (uint8_t i = 0; i != NUM_AXIS; ++i) {
        axisData[i].residual = (timers[i] ? timers[i] - next : 0);
    }

    return next;
}

void Isr() {}

} // namespace motion
} // namespace modules
