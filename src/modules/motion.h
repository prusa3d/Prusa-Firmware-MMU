#pragma once
#include <stdint.h>

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

namespace modules {
namespace motion {

enum Axis {
    Pulley,
    Selector,
    Idler,
};

enum MotorMode {
    Stealth,
    Normal
};

enum IdlerMode {
    Engage,
    Disengage
};

/// As step and dir pins are randomly scattered on the board for each of the axes/motors
/// it is convenient to make a common interface for them
class StepDirPins {
public:
    static void SetIdlerDirUp();
    static void SetIdlerDirDown();

    static void SetSelectorDirLeft();
    static void SetSelectorDirRight();

    static void SetPulleyDirPull();
    static void SetPulleyDirPush();

    static void StepIdler(uint8_t on);
    static void StepSelector(uint8_t on);
    static void StepPulley(uint8_t on);
};

/// @@TODO this is subject of discussion and change in the future
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

    /// Enqueue performing of homing of an axis
    /// @@TODO
    void Home(Axis axis, bool direction);

    /// Set mode of TMC/motors operation
    /// Common for all axes/motors
    void SetMode(MotorMode mode);

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
