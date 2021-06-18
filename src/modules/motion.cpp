#include "motion.h"
#include "../hal/shr16.h"

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

void Motion::SetMode(MotorMode mode) {}

void Motion::Step() {}

bool Motion::QueueEmpty() const { return false; }

void Motion::AbortPlannedMoves() {}

void ISR() {}

//@@TODO check the directions
void StepDirPins::SetIdlerDirUp() {
    hal::shr16::shr16.SetTMCDir(Axis::Idler, true);
}

void StepDirPins::SetIdlerDirDown() {
    hal::shr16::shr16.SetTMCDir(Axis::Idler, false);
}

void StepDirPins::SetSelectorDirLeft() {
    hal::shr16::shr16.SetTMCDir(Axis::Selector, true);
}
void StepDirPins::SetSelectorDirRight() {
    hal::shr16::shr16.SetTMCDir(Axis::Selector, false);
}

void StepDirPins::SetPulleyDirPull() {
    hal::shr16::shr16.SetTMCDir(Axis::Pulley, true);
}
void StepDirPins::SetPulleyDirPush() {
    hal::shr16::shr16.SetTMCDir(Axis::Pulley, false);
}

void StepDirPins::StepIdler(uint8_t on) {
    // PORTD |= idler_step_pin;
}

void StepDirPins::StepSelector(uint8_t on) {
    // PORTD |= selector_step_pin;
}

void StepDirPins::StepPulley(uint8_t on) {
    // PORTB |= pulley_step_pin;
}

} // namespace motion
} // namespace modules
