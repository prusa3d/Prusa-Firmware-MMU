#include "selector.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"

namespace modules {
namespace selector {

// @@TODO PROGMEM
const uint16_t Selector::slotPositions[slotPositionSize] = { 1, 2, 3, 4, 5, 6 }; // @@TODO

Selector selector;

namespace mm = modules::motion;

bool Selector::MoveToSlot(uint8_t slot) {
    if (state == Moving)
        return false;

    plannedSlot = slot;

    if (currentSlot == slot)
        return true;

    mm::motion.InitAxis(mm::Selector);
    mm::motion.PlanMove(mm::Selector, slotPositions[slot] - mm::motion.CurrentPos(mm::Selector), 1000); // @@TODO
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
        mm::motion.DisableAxis(mm::Selector); // turn off selector motor's power every time
        return true;
    case Failed:
    default:
        return true;
    }
}

} // namespace selector
} // namespace modules
