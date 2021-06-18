#include "idler.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"

namespace modules {
namespace idler {

// @@TODO PROGMEM
uint16_t const Idler::slotPositions[6] = { 1, 2, 3, 4, 5, 0 };

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
    mm::motion.PlanMove(mm::Idler, slotPositions[5] - mm::motion.CurrentPos(mm::Idler), 1000); // @@TODO
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
    mm::motion.PlanMove(mm::Idler, slotPositions[slot] - mm::motion.CurrentPos(mm::Idler), 1000); // @@TODO
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
            mm::motion.DisableAxis(mm::Idler);

        return true;
    case Failed:
    default:
        return true;
    }
}

} // namespace idler
} // namespace modules
