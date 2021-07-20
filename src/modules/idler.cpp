#include "idler.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"

namespace modules {
namespace idler {

Idler idler;

namespace mm = modules::motion;

Idler::EngageDisengage Idler::Disengage() {
    if (state == Moving)
        return EngageDisengage::Refused;

    plannedEngage = false;

    if (!Engaged())
        return true;

    mm::motion.InitAxis(mm::Idler);
    // plan move to idle position
    mm::motion.PlanMoveTo<mm::Idler>(SlotPosition(IdleSlotIndex()), 1000._I_deg_s); // @@TODO
    state = Moving;
    return true;
//        return EngageDisengage::Accepted;
//
//    if (!mm::motion.InitAxis(mm::Idler)) {
//        state = Failed;
//        return EngageDisengage::Failed;
//    } else {
//        // plan move to idle position
//        mm::motion.PlanMove(mm::Idler, config::idlerSlotPositions[IdleSlotIndex()] - mm::motion.Position(mm::Idler), 1000); // @@TODO
//        state = Moving;
//        return EngageDisengage::Accepted;
//    }
}

Idler::EngageDisengage Idler::Engage(uint8_t slot) {
    if (state == Moving)
        return EngageDisengage::Refused;

    plannedSlot = slot;
    plannedEngage = true;

    if (Engaged())
        return true;

    mm::motion.InitAxis(mm::Idler);
    mm::motion.PlanMoveTo<mm::Idler>(SlotPosition(slot), 1000._I_deg_s); // @@TODO
    state = Moving;
    return true;
//        return EngageDisengage::Accepted;
//
//    if (!mm::motion.InitAxis(mm::Idler)) {
//        state = Failed;
//        return EngageDisengage::Failed;
//    } else {
//        mm::motion.PlanMove(mm::Idler, config::idlerSlotPositions[slot] - mm::motion.Position(mm::Idler), 1000); // @@TODO
//        state = Moving;
//        return EngageDisengage::Accepted;
//    }
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

} // namespace idler
} // namespace modules
