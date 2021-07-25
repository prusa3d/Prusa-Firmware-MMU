#include "selector.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"

namespace modules {
namespace selector {

Selector selector;

namespace mm = modules::motion;

bool Selector::MoveToSlot(uint8_t slot) {
    if (state == Moving)
        return false;

    plannedSlot = slot;

    if (currentSlot == slot)
        return true;

    mm::motion.InitAxis(mm::Selector);
    mm::motion.PlanMoveTo<mm::Selector>(SlotPosition(slot), 1000.0_S_mm_s); // @@TODO
    state = Moving;
    return true;
}

bool Selector::Home() {
    if (state == Moving)
        return false;
    mm::motion.InitAxis(mm::Selector);
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
        mm::motion.Disable(mm::Selector); // turn off selector motor's power every time
        return true;
    case Failed:
    default:
        return true;
    }
}

} // namespace selector
} // namespace modules
