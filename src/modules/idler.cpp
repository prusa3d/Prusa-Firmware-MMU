#include "idler.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"

namespace modules {
namespace idler {

Idler idler;

void Idler::PrepareMoveToPlannedSlot() {
    mm::motion.PlanMoveTo<mm::Idler>(SlotPosition(plannedSlot), mm::unitToAxisUnit<mm::I_speed_t>(config::idlerFeedrate));
}

void Idler::PlanHomingMove() {
    mm::motion.PlanMove<mm::Idler>(mm::unitToAxisUnit<mm::I_pos_t>(-config::idlerLimits.lenght * 2), mm::unitToAxisUnit<mm::I_speed_t>(config::idlerFeedrate));
}

Idler::OperationResult Idler::Disengage() {
    if (state == Moving)
        return OperationResult::Refused;

    plannedSlot = IdleSlotIndex();
    plannedEngage = false;

    if (!Engaged())
        return OperationResult::Accepted;

    return InitMovement(mm::Idler);
}

Idler::OperationResult Idler::Engage(uint8_t slot) {
    if (state == Moving)
        return OperationResult::Refused;

    plannedSlot = slot;
    plannedEngage = true;

    if (Engaged())
        return OperationResult::Accepted;

    return InitMovement(mm::Idler);
}

bool Idler::Home() {
    if (state == Moving)
        return false;
    plannedEngage = false;
    PlanHome(mm::Idler);
    return true;
}

bool Idler::Step() {
    switch (state) {
    case Moving:
        PerformMove(mm::Idler);
        return false;
    case Homing:
        PerformHome(mm::Idler);
        return false;
    case Ready:
        currentlyEngaged = plannedEngage;
        currentSlot = plannedSlot;

        if (!Engaged()) // turn off power into the Idler motor when disengaged (no force necessary)
            mm::motion.Disable(mm::Idler);

        return true;
    case Failed:
    default:
        return true;
    }
}

} // namespace idler
} // namespace modules
