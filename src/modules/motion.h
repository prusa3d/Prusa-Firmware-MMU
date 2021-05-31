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
    Idler,
    Selector,
    Pulley
};

enum Mode {
    Stealth,
    Normal
};

class Motion {
public:
    /// Init axis driver
    void InitAxis(Axis axis) {}

    /// Disable axis motor
    void DisableAxis(Axis axis) {}

    /// Enqueue move of a specific motor/axis into planner buffer
    void PlanMove(Axis axis, float targetPosition, uint16_t feedrate);

    /// Enqueue performing of homing of an axis
    void Home(Axis axis);

    /// Set mode of TMC/motors operation
    /// Common for all axes/motors
    void SetMode(Mode mode);

    /// State machine doing all the planning and stepping preparation based on received commands
    void Step();

    /// probably higher-level operations knowing the semantic meaning of axes
    enum IdlerMode { Engage,
        Disengage };
    void Idler(IdlerMode im) {}

private:
};

/// ISR stepping routine
extern void ISR();

extern Motion motion;

} // namespace motion
} // namespace modules
