#include "idler.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"

namespace modules {
namespace idler {

Idler idler;

namespace mm = modules::motion;

bool Idler::Disengage() {
    if (state == Moving)
        return false;

    plannedEngage = false;

    if (!Engaged())
        return true;

    mm::motion.InitAxis(mm::Idler);
    // plan move to idle position
    mm::motion.PlanMoveTo<mm::Idler>(SlotPosition(IdleSlotIndex()), 1000._I_deg_s); // @@TODO
    state = Moving;
    return true;
}

bool Idler::Engage(uint8_t slot) {
    if (state == Moving)
        return false;

    plannedSlot = slot;
    plannedEngage = true;

    if (Engaged())
        return true;

    mm::motion.InitAxis(mm::Idler);
    mm::motion.PlanMoveTo<mm::Idler>(SlotPosition(slot), 1000._I_deg_s); // @@TODO
    state = Moving;
    return true;
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
