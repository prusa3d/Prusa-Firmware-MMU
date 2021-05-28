#include "motion.h"

namespace modules {
namespace motion {

Motion motion;

void Motion::PlanMove(Axis axis, float targetPosition, uint16_t feedrate) {}

void Motion::Home(Axis axis) {}

void Motion::SetMode(Mode mode) {}

void Motion::Step() {}

void ISR() {}

} // namespace motion
} // namespace modules
