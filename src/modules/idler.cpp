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
    mm::motion.InitAxis(mm::Idler);
    mm::motion.Home(mm::Idler, false);
    return true;
}

bool Idler::Step() {
    switch (state) {
    case Moving:
        if (mm::motion.QueueEmpty()) { //@@TODO this will block until all axes made their movements,
            // not sure if that is something we want
            // move finished
            state = Ready;
        }
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

//hal::tmc2130::MotorParams Idler::TMCDriverParams() const {
//    return

//}

} // namespace idler
} // namespace modules
