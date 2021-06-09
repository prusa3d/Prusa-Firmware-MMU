#include "selector.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"

namespace modules {
namespace selector {

namespace mm = modules::motion;

bool Selector::MoveToSlot(uint8_t slot) {
    if (state == Moving)
        return false;
    plannedSlot = slot;
    // mm::motion.PlanMove(1, slotPositions[slot], 0, 1000, 0, 0); // @@TODO
    state = Moving;
    return true;
}

bool Selector::Home() {
    if (state == Moving)
        return false;
    mm::motion.Home(mm::Selector, false);
    return true;
}

bool Selector::Step() {
    switch (state) {
    case Moving:
        if (mm::motion.QueueEmpty()) {
            // move finished
            state = Ready;
        }
        return false;
    case Ready:
        currentSlot = plannedSlot;
        return true;
    case Failed:
    default:
        return true;
    }
}

} // namespace selector
} // namespace modules
