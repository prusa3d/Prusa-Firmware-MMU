#pragma once
#include "../pins.h"
#include "pulse_gen.h"

namespace modules {

/// @@TODO
/// Logic of motor handling
/// Ideally enable stepping of motors under ISR (all timers have higher priority than serial)
namespace motion {

using namespace hal::tmc2130;
using pulse_gen::pos_t;
using pulse_gen::steps_t;

/// Main axis enumeration
enum Axis : uint8_t {
    Pulley,
    Selector,
    Idler,
    _Axis_Last = Idler
};

static constexpr uint8_t NUM_AXIS = _Axis_Last + 1;

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
        .params = { .idx = Pulley, .dirOn = config::pulley.dirOn, .csPin = PULLEY_CS_PIN, .stepPin = PULLEY_STEP_PIN, .sgPin = PULLEY_SG_PIN, .uSteps = config::pulley.uSteps },
        .currents = { .vSense = config::pulley.vSense, .iRun = config::pulley.iRun, .iHold = config::pulley.iHold },
        .mode = DefaultMotorMode(config::pulley),
        .jerk = config::pulley.jerk,
        .accel = config::pulley.accel,
    },
    // Selector
    {
        .name = 'S',
        .params = { .idx = Selector, .dirOn = config::selector.dirOn, .csPin = SELECTOR_CS_PIN, .stepPin = SELECTOR_STEP_PIN, .sgPin = SELECTOR_SG_PIN, .uSteps = config::selector.uSteps },
        .currents = { .vSense = config::selector.vSense, .iRun = config::selector.iRun, .iHold = config::selector.iHold },
        .mode = DefaultMotorMode(config::selector),
        .jerk = config::selector.jerk,
        .accel = config::selector.accel,
    },
    // Idler
    {
        .name = 'I',
        .params = { .idx = Idler, .dirOn = config::idler.dirOn, .csPin = IDLER_CS_PIN, .stepPin = IDLER_STEP_PIN, .sgPin = IDLER_SG_PIN, .uSteps = config::idler.uSteps },
        .currents = { .vSense = config::idler.vSense, .iRun = config::idler.iRun, .iHold = config::idler.iHold },
        .mode = DefaultMotorMode(config::idler),
        .jerk = config::idler.jerk,
        .accel = config::idler.accel,
    },
};

class Motion {
public:
    inline constexpr Motion() = default;

    /// Init axis driver - @@TODO this should be probably hidden
    /// somewhere deeper ... something should manage the axes and their
    /// state especially when the TMC may get randomly reset (deinited)
    void InitAxis(Axis axis);

    /// Disable axis motor
    void DisableAxis(Axis axis);

    /// @returns true if a stall guard event occurred recently on the axis
    bool StallGuard(Axis axis);

    /// clear stall guard flag reported on an axis
    void ClearStallGuardFlag(Axis axis);

    /// Enqueue a single axis move in steps starting and ending at zero speed with maximum
    /// feedrate. Moves can only be enqueued if the axis is not Full().
    /// @param axis axis affected
    /// @param pos target position
    /// @param feedrate maximum feedrate
    void PlanMoveTo(Axis axis, pos_t pos, steps_t feedrate);

    /// Enqueue a single axis move in steps starting and ending at zero speed with maximum
    /// feedrate. Moves can only be enqueued if the axis is not Full().
    /// @param axis axis affected
    /// @param delta relative to current position
    /// @param feedrate maximum feedrate
    void PlanMove(Axis axis, pos_t delta, steps_t feedrate) {
        PlanMoveTo(axis, CurrentPos(axis) + delta, feedrate);
    }

    /// @returns current position of an axis
    /// @param axis axis affected
    pos_t CurrentPos(Axis axis) const;

    /// Set acceleration for the selected axis
    /// @param axis axis affected
    /// @param accel acceleration
    void SetAcceleration(Axis axis, steps_t accel) {
        axisData[axis].ctrl.SetAcceleration(accel);
    }

    /// Enqueue performing of homing of an axis
    void Home(Axis axis, bool direction);

    /// Set mode of TMC/motors operation
    void SetMode(Axis axis, MotorMode mode);

    /// State machine doing all the planning and stepping preparation based on received commands
    void Step();

    /// @returns true if all planned moves have been finished for all axes
    bool QueueEmpty() const;

    /// @returns true if all planned moves have been finished for one axis
    /// @param axis requested
    bool QueueEmpty(Axis axis) const { return axisData[axis].ctrl.QueueEmpty(); }

    /// @returns false if new moves can still be planned for _any_ axis
    bool Full() const {
        for (uint8_t i = 0; i != NUM_AXIS; ++i)
            if (axisData[i].ctrl.Full())
                return true;
        return false;
    }

    /// @returns false if new moves can still be planned for one axis
    /// @param axis axis requested
    bool Full(Axis axis) const { return axisData[axis].ctrl.Full(); }

    /// stop whatever moves are being done
    void AbortPlannedMoves();

private:
    struct AxisData {
        TMC2130 drv; ///< Motor driver
        pulse_gen::PulseGen ctrl; ///< Motor controller
    };

    /// Helper to initialize AxisData members
    static AxisData DataForAxis(Axis axis) {
        return {
            .drv = { axisParams[axis].params, axisParams[axis].currents, axisParams[axis].mode },
            .ctrl = { axisParams[axis].jerk, axisParams[axis].accel },
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
extern void ISR();

extern Motion motion;

} // namespace motion
} // namespace modules
