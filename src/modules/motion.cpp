#include "motion.h"

// TODO: use proper timer abstraction
#ifdef __AVR__
#include <avr/interrupt.h>
#else
//#include "../hal/timers.h"
#endif

namespace modules {
namespace motion {

Motion motion;

bool Motion::InitAxis(Axis axis) {
    // disable the axis and re-init the driver: this will clear the internal
    // StallGuard data as a result without special handling
    Disable(axis);
    return axisData[axis].drv.Init(axisParams[axis].params, axisParams[axis].currents, axisParams[axis].mode);
}

void Motion::SetEnabled(Axis axis, bool enabled) {
    if (enabled != axisData[axis].enabled) {
        axisData[axis].drv.SetEnabled(axisParams[axis].params, enabled);
        axisData[axis].enabled = enabled;
    } // else skip unnecessary Enable/Disable operations on an axis if already in the desired state
}

void Motion::SetMode(Axis axis, MotorMode mode) {
    for (uint8_t i = 0; i != NUM_AXIS; ++i)
        axisData[axis].drv.SetMode(axisParams[axis].params, mode);
}

bool Motion::StallGuard(Axis axis) {
    return axisData[axis].drv.Stalled();
}

void Motion::StallGuardReset(Axis axis) {
    axisData[axis].drv.ClearStallguard(axisParams[axis].params);
}

// TODO: not implemented
void Motion::Home(Axis axis, bool direction) {
}

void Motion::PlanMoveTo(Axis axis, pos_t pos, steps_t feed_rate, steps_t end_rate) {
    if (axisData[axis].ctrl.PlanMoveTo(pos, feed_rate, end_rate)) {
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

void Motion::AbortPlannedMoves(bool halt) {
    for (uint8_t i = 0; i != NUM_AXIS; ++i)
        axisData[i].ctrl.AbortPlannedMoves(halt);
}

st_timer_t Motion::Step() {
    st_timer_t timers[NUM_AXIS];

    // step and calculate interval for each new move
    for (uint8_t i = 0; i != NUM_AXIS; ++i) {
        timers[i] = axisData[i].residual;
        if (timers[i] <= config::stepTimerQuantum) {
            if (timers[i] || !axisData[i].ctrl.QueueEmpty()) {
                if (st_timer_t next = axisData[i].ctrl.Step(axisParams[i].params)) {
                    timers[i] += next;

                    // axis has been moved, run the tmc2130 Isr for this axis
                    axisData[i].drv.Isr(axisParams[i].params);
                }
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

static inline void Isr() {
#ifdef __AVR__
    st_timer_t next = motion.Step();
    // TODO: use proper timer abstraction
    if (next)
        OCR1A = next;
    else {
        // Idling: plan the next interrupt after 8ms from now.
        OCR1A = 0x4000;
    }
#endif
}

void Init() {
#ifdef __AVR__
    // TODO: use proper timer abstraction

    // waveform generation = 0100 = CTC
    TCCR1B &= ~(1 << WGM13);
    TCCR1B |= (1 << WGM12);
    TCCR1A &= ~(1 << WGM11);
    TCCR1A &= ~(1 << WGM10);

    // output mode = 00 (disconnected)
    TCCR1A &= ~(3 << COM1A0);
    TCCR1A &= ~(3 << COM1B0);

    // Set the timer pre-scaler
    // We use divider of 8, resulting in a 2MHz timer frequency on a 16MHz MCU
    TCCR1B = (TCCR1B & ~(0x07 << CS10)) | (2 << CS10);

    // Plan the first interrupt after 8ms from now.
    OCR1A = 0x4000;
    TCNT1 = 0;

    // Enable interrupt
    TIMSK1 |= (1 << OCIE1A);
#endif
}

} // namespace motion
} // namespace modules

#ifdef __AVR__
ISR(TIMER1_COMPA_vect) {
    modules::motion::Isr();
}
#endif
