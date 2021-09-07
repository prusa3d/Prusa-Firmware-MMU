#include "selector.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"

namespace modules {
namespace selector {

Selector selector;

void Selector::PrepareMoveToPlannedSlot() {
    mm::motion.PlanMoveTo<mm::Selector>(SlotPosition(plannedSlot), mm::unitToAxisUnit<mm::S_speed_t>(config::selectorFeedrate));
}

void Selector::PlanHomingMove() {
    mm::motion.PlanMove<mm::Selector>(mm::unitToAxisUnit<mm::S_pos_t>(-100.0_mm), mm::unitToAxisUnit<mm::S_speed_t>(config::selectorFeedrate));
}

Selector::OperationResult Selector::MoveToSlot(uint8_t slot) {
    if (state == Moving)
        return OperationResult::Refused;

    plannedSlot = slot;

    if (currentSlot == slot)
        return OperationResult::Accepted;

    return InitMovement(mm::Selector);
}

bool Selector::Home() {
    if (state == Moving)
        return false;
    PlanHome(mm::Selector);
    return true;
}

bool Selector::Step() {
    switch (state) {
    case Moving:
        PerformMove(mm::Selector);
        return false;
    case Homing:
        PerformHome(mm::Selector);
        return false;
    case Ready:
        currentSlot = plannedSlot;
        mm::motion.Disable(mm::Selector); // turn off selector motor's power every time
        return true;
    case Failed:
    default:
        return true;
    }
}

} // namespace selector
} // namespace modules
