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

    if (!Engaged())
        return true;

    plannedEngage = false;
    mm::motion.InitAxis(mm::Idler);
    // plan move to idle position
    // mm::motion.PlanMove(0, idle_position, 0, 1000, 0, 0); // @@TODO
    state = Moving;
    return true;
}

bool Idler::Engage(uint8_t slot) {
    if (state == Moving)
        return false;

    if (Engaged())
        return true;

    plannedSlot = slot;
    plannedEngage = true;
    mm::motion.InitAxis(mm::Idler);
    // mm::motion.PlanMove(0, slotPositions[slot], 0, 1000, 0, 0); // @@TODO
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
        if (mm::motion.QueueEmpty()) {
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
