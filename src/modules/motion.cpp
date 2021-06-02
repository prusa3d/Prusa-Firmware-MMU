#include "motion.h"
#include "../hal/shr16.h"

namespace modules {
namespace motion {

Motion motion;

void Motion::PlanMove(Axis axis, float targetPosition, uint16_t feedrate) {}

void Motion::Home(Axis axis) {}

void Motion::SetMode(MotorMode mode) {}

void Motion::Step() {}

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
