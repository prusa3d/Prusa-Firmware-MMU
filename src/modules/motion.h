#pragma once
#include <stdint.h>
#include "../config/config.h"
#include "../hal/tmc2130.h"
#include "../pins.h"

namespace modules {

/// @@TODO
/// Logic of motor handling
/// Ideally enable stepping of motors under ISR (all timers have higher priority than serial)

/// input:
/// motor, direction, speed (step rate), may be acceleration if necessary (not sure)
/// enable/disable motor current
/// stealth/normal

/// Motors:
/// idler
/// selector
/// pulley

/// Operations:
/// setDir();
/// rotate(speed)
/// rotate(speed, angle/steps)
/// home?
namespace motion {

/// Main axis enumeration
enum Axis : uint8_t {
    Pulley,
    Selector,
    Idler,
    _Axis_Last = Idler
};

static constexpr uint8_t NUM_AXIS = _Axis_Last + 1;

/// Static axis configuration
struct AxisParams {
    char name;
    hal::tmc2130::MotorParams params;
    hal::tmc2130::MotorCurrents currents;
    float scale;
    float accel;
};

static constexpr AxisParams axisParams[NUM_AXIS] = {
    // Idler
    {
        .name = 'I',
        .params = {
            .idx = Idler,
            .dirOn = config::idler.dirOn,
            .csPin = IDLER_CS_PIN,
            .stepPin = IDLER_STEP_PIN,
            .sgPin = IDLER_SG_PIN,
            .uSteps = config::idler.uSteps },
        .currents = { .vSense = config::idler.vSense, .iRun = config::idler.iRun, .iHold = config::idler.iHold },
        .scale = config::idler.scale,
        .accel = config::idler.accel,
    },
    // Pulley
    {
        .name = 'P',
        .params = { .idx = Pulley, .dirOn = config::pulley.dirOn, .csPin = PULLEY_CS_PIN, .stepPin = PULLEY_STEP_PIN, .sgPin = PULLEY_SG_PIN, .uSteps = config::pulley.uSteps },
        .currents = { .vSense = config::pulley.vSense, .iRun = config::pulley.iRun, .iHold = config::pulley.iHold },
        .scale = config::pulley.scale,
        .accel = config::pulley.accel,
    },
    // Selector
    {
        .name = 'S',
        .params = { .idx = Selector, .dirOn = config::selector.dirOn, .csPin = SELECTOR_CS_PIN, .stepPin = SELECTOR_STEP_PIN, .sgPin = SELECTOR_SG_PIN, .uSteps = config::selector.uSteps },
        .currents = { .vSense = config::selector.vSense, .iRun = config::selector.iRun, .iHold = config::selector.iHold },
        .scale = config::selector.scale,
        .accel = config::selector.accel,
    },
};

enum IdlerMode : uint8_t {
    Engage,
    Disengage
};

class Motion {
public:
    inline constexpr Motion() = default;

    /// Init axis driver - @@TODO this should be probably hidden somewhere deeper ... something should manage the axes and their state
    /// especially when the TMC may get randomly reset (deinited)
    void InitAxis(Axis axis);

    /// Disable axis motor
    void DisableAxis(Axis axis);

    /// @returns true if a stall guard event occurred recently on the axis
    bool StallGuard(Axis axis);

    /// clear stall guard flag reported on an axis
    void ClearStallGuardFlag(Axis axis);

    /// Enqueue move of a specific motor/axis into planner buffer
    /// @param pulley, idler, selector - target coords
    void PlanMove(int16_t pulley, int16_t idler, int16_t selector, uint16_t feedrate, uint16_t starting_speed, uint16_t ending_speed);

    /// Enqueue a single axis move in steps starting and ending at zero speed with maximum feedrate
    /// @param axis axis affected
    /// @param delta number of steps in either direction
    /// @param feedrate maximum feedrate/speed after acceleration
    void PlanMove(Axis axis, int16_t delta, uint16_t feedrate);

    /// @returns current position of an axis
    /// @param axis axis affected
    uint16_t CurrentPos(Axis axis) const;

    /// Enqueue performing of homing of an axis
    /// @@TODO
    void Home(Axis axis, bool direction);

    /// Set mode of TMC/motors operation
    /// Common for all axes/motors
    void SetMode(hal::tmc2130::MotorMode mode);

    /// State machine doing all the planning and stepping preparation based on received commands
    void Step();

    /// @returns true if all planned moves have been finished
    bool QueueEmpty() const;

    /// stop whatever moves are being done
    void AbortPlannedMoves();

    /// probably higher-level operations knowing the semantic meaning of axes

private:
};

/// ISR stepping routine
extern void ISR();

extern Motion motion;

} // namespace motion
} // namespace modules
