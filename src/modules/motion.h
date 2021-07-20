#pragma once
#include "../pins.h"
#include "pulse_gen.h"
#include "axisunit.h"

namespace modules {

/// @@TODO
/// Logic of motor handling
/// Ideally enable stepping of motors under ISR (all timers have higher priority than serial)
namespace motion {

// Import axes definitions
using config::NUM_AXIS;

using namespace hal::tmc2130;
using pulse_gen::st_timer_t;

// Check for configuration invariants
static_assert(
    (1. / (F_CPU / config::stepTimerFrequencyDivider) * config::stepTimerQuantum)
        < (1. / config::maxStepFrequency / 2),
    "stepTimerQuantum must be smaller than the maximal stepping frequency interval");

/// Main axis enumeration
struct AxisParams {
    char name;
    MotorParams params;
    MotorCurrents currents;
    MotorMode mode;
    steps_t jerk;
    steps_t accel;
};

/// Return the default motor mode for an Axis
static constexpr MotorMode DefaultMotorMode(const config::AxisConfig &axis) {
    return axis.stealth ? MotorMode::Stealth : MotorMode::Normal;
}

/// Static axis configuration
static constexpr AxisParams axisParams[NUM_AXIS] = {
    // Pulley
    {
        .name = 'P',
        .params = { .spi = hal::spi::TmcSpiBus, .idx = Pulley, .dirOn = config::pulley.dirOn, .csPin = PULLEY_CS_PIN, .stepPin = PULLEY_STEP_PIN, .sgPin = PULLEY_SG_PIN, .uSteps = config::pulley.uSteps },
        .currents = { .vSense = config::pulley.vSense, .iRun = config::pulley.iRun, .iHold = config::pulley.iHold },
        .mode = DefaultMotorMode(config::pulley),
        .jerk = unitToSteps<P_speed_t>(config::pulleyLimits.jerk),
        .accel = unitToSteps<P_accel_t>(config::pulleyLimits.accel),
    },
    // Selector
    {
        .name = 'S',
        .params = { .spi = hal::spi::TmcSpiBus, .idx = Selector, .dirOn = config::selector.dirOn, .csPin = SELECTOR_CS_PIN, .stepPin = SELECTOR_STEP_PIN, .sgPin = SELECTOR_SG_PIN, .uSteps = config::selector.uSteps },
        .currents = { .vSense = config::selector.vSense, .iRun = config::selector.iRun, .iHold = config::selector.iHold },
        .mode = DefaultMotorMode(config::selector),
        .jerk = unitToSteps<S_speed_t>(config::selectorLimits.jerk),
        .accel = unitToSteps<S_accel_t>(config::selectorLimits.accel),
    },
    // Idler
    {
        .name = 'I',
        .params = { .spi = hal::spi::TmcSpiBus, .idx = Idler, .dirOn = config::idler.dirOn, .csPin = IDLER_CS_PIN, .stepPin = IDLER_STEP_PIN, .sgPin = IDLER_SG_PIN, .uSteps = config::idler.uSteps },
        .currents = { .vSense = config::idler.vSense, .iRun = config::idler.iRun, .iHold = config::idler.iHold },
        .mode = DefaultMotorMode(config::idler),
        .jerk = unitToSteps<I_speed_t>(config::idlerLimits.jerk),
        .accel = unitToSteps<I_accel_t>(config::idlerLimits.accel),
    },
};

class Motion {
public:
    inline constexpr Motion() = default;

    /// Init axis driver - @@TODO this should be probably hidden
    /// somewhere deeper ... something should manage the axes and their
    /// state especially when the TMC may get randomly reset (deinited)
    /// @returns true if the init was successful (TMC2130 responded ok)
    bool InitAxis(Axis axis);

    /// Set axis power status. One must manually ensure no moves are currently being
    /// performed by calling QueueEmpty().
    void SetEnabled(Axis axis, bool enabled);

    /// Disable axis motor. One must manually ensure no moves are currently being
    /// performed by calling QueueEmpty().
    void Disable(Axis axis) { SetEnabled(axis, false); }

    /// Set mode of TMC/motors operation. One must manually ensure no moves are currently
    /// being performed by calling QueueEmpty().
    void SetMode(Axis axis, MotorMode mode);

    /// @returns true if a stall guard event occurred recently on the axis
    bool StallGuard(Axis axis);

    /// clear stall guard flag reported on an axis
    void StallGuardReset(Axis axis);

    /// Enqueue performing of homing of an axis
    void Home(Axis axis, bool direction);

    /// Enqueue a single axis move in steps starting and ending at zero speed with maximum
    /// feedrate. Moves can only be enqueued if the axis is not Full().
    /// @param axis axis affected
    /// @param pos target position
    /// @param feedrate maximum feedrate
    void PlanMoveTo(Axis axis, pos_t pos, steps_t feedrate);

    /// Enqueue a single axis move using PlanMoveTo, but using AxisUnit. The Axis needs to
    /// be supplied as the first template argument: PlanMoveTo<axis>(pos, rate).
    /// @see PlanMoveTo, unitToSteps
    template <Axis A>
    constexpr void PlanMoveTo(AxisUnit<pos_t, A, Lenght> pos, AxisUnit<steps_t, A, Speed> feedrate) {
        PlanMoveTo(A, pos.v, feedrate.v);
    }

    /// Enqueue a single axis move using PlanMoveTo, but using physical units. The Axis
    /// needs to be supplied as the first template argument: PlanMoveTo<axis>(pos, rate).
    /// @see PlanMoveTo, unitToSteps
    template <Axis A, config::UnitBase B>
    constexpr void PlanMoveTo(config::Unit<long double, B, Lenght> pos,
        config::Unit<long double, B, Speed> feedrate) {
        PlanMoveTo<A>(
            unitToAxisUnit<AxisUnit<pos_t, A, Lenght>>(pos),
            unitToAxisUnit<AxisUnit<steps_t, A, Speed>>(feedrate));
    }

    /// Enqueue a single axis move in steps starting and ending at zero speed with maximum
    /// feedrate. Moves can only be enqueued if the axis is not Full().
    /// @param axis axis affected
    /// @param delta relative to current position
    /// @param feedrate maximum feedrate
    void PlanMove(Axis axis, pos_t delta, steps_t feedrate) {
        PlanMoveTo(axis, Position(axis) + delta, feedrate);
    }

    /// Enqueue a single axis move using PlanMove, but using AxisUnit. The Axis needs to
    /// be supplied as the first template argument: PlanMove<axis>(pos, rate).
    /// @see PlanMove, unitToSteps
    template <Axis A>
    constexpr void PlanMove(AxisUnit<pos_t, A, Lenght> delta, AxisUnit<steps_t, A, Speed> feedrate) {
        PlanMove(A, delta.v, feedrate.v);
    }

    /// Enqueue a single axis move using PlanMove, but using physical units. The Axis needs to
    /// be supplied as the first template argument: PlanMove<axis>(pos, rate).
    /// @see PlanMove, unitToSteps
    template <Axis A, config::UnitBase B>
    constexpr void PlanMove(config::Unit<long double, B, Lenght> delta,
        config::Unit<long double, B, Speed> feedrate) {
        PlanMove<A>(
            unitToAxisUnit<AxisUnit<pos_t, A, Lenght>>(delta),
            unitToAxisUnit<AxisUnit<steps_t, A, Speed>>(feedrate));
    }

    /// @returns head position of an axis (last enqueued position)
    /// @param axis axis affected
    pos_t Position(Axis axis) const;

    /// Fetch the current position of the axis while stepping. This function is expensive!
    /// It's necessary only in exceptional cases. For regular usage, Position() should
    /// probably be used instead.
    /// @param axis axis affected
    /// @returns the current position of the axis
    pos_t CurPosition(Axis axis) const { return axisData[axis].ctrl.CurPosition(); }

    /// Set the position of an axis. Should only be called when the queue is empty.
    /// @param axis axis affected
    /// @param x position to set
    void SetPosition(Axis axis, pos_t x) { axisData[axis].ctrl.SetPosition(x); }

    /// Get current acceleration for the selected axis
    /// @param axis axis affected
    /// @returns acceleration
    steps_t Acceleration(Axis axis) const {
        return axisData[axis].ctrl.Acceleration();
    }

    /// Set acceleration for the selected axis
    /// @param axis axis affected
    /// @param accel acceleration
    void SetAcceleration(Axis axis, steps_t accel) {
        axisData[axis].ctrl.SetAcceleration(accel);
    }

    /// Get current jerk for the selected axis
    /// @param axis axis affected
    /// @returns jerk
    steps_t Jerk(Axis axis) const {
        return axisData[axis].ctrl.Jerk();
    }

    /// Set maximum jerk for the selected axis
    /// @param axis axis affected
    /// @param max_jerk maximum jerk
    void SetJerk(Axis axis, steps_t max_jerk) {
        return axisData[axis].ctrl.SetJerk(max_jerk);
    }

    /// State machine doing all the planning and stepping. Called by the stepping ISR.
    /// @returns the interval for the next tick
    st_timer_t Step();

    /// @returns true if all planned moves have been finished for all axes
    bool QueueEmpty() const;

    /// @returns true if all planned moves have been finished for one axis
    /// @param axis requested
    bool QueueEmpty(Axis axis) const { return axisData[axis].ctrl.QueueEmpty(); }

    /// @returns false if new moves can still be planned for one axis
    /// @param axis axis requested
    bool Full(Axis axis) const { return axisData[axis].ctrl.Full(); }

    /// stop whatever moves are being done
    void AbortPlannedMoves();

private:
    struct AxisData {
        TMC2130 drv; ///< Motor driver
        pulse_gen::PulseGen ctrl; ///< Motor controller
        bool enabled; ///< Axis enabled
        st_timer_t residual; ///< Axis timer residual
    };

    /// Helper to initialize AxisData members
    static AxisData DataForAxis(Axis axis) {
        return {
            .drv = {
                axisParams[axis].params,
                axisParams[axis].currents,
                axisParams[axis].mode,
            },
            .ctrl = {
                axisParams[axis].jerk,
                axisParams[axis].accel,
            },
            .enabled = false
        };
    }

    /// Dynamic axis data
    AxisData axisData[NUM_AXIS] = {
        DataForAxis(Pulley),
        DataForAxis(Selector),
        DataForAxis(Idler),
    };
};

/// ISR stepping routine
//extern void Isr();

extern Motion motion;

} // namespace motion
} // namespace modules
