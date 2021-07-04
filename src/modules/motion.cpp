#include "motion.h"

namespace modules {
namespace motion {

Motion motion;

void Motion::InitAxis(Axis axis) {}

void Motion::DisableAxis(Axis axis) {}

bool Motion::StallGuard(Axis axis) { return false; }

void Motion::ClearStallGuardFlag(Axis axis) {}

void Motion::PlanMove(int16_t pulley, int16_t idler, int16_t selector, uint16_t feedrate, uint16_t starting_speed, uint16_t ending_speed) {}

void Motion::PlanMove(Axis axis, int16_t delta, uint16_t feedrate) {}

uint16_t Motion::CurrentPos(Axis axis) const { return 0; }

void Motion::Home(Axis axis, bool direction) {}

void Motion::SetMode(hal::tmc2130::MotorMode mode) {}

void Motion::Step() {}

bool Motion::QueueEmpty() const { return false; }

void Motion::AbortPlannedMoves() {}

void ISR() {}

} // namespace motion
} // namespace modules
