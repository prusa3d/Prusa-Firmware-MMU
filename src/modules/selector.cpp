#include "selector.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"
#ifdef DEBUG_LOGIC
#include "../hal/usart.h"
#include <string.h>
#include <stdio.h>
#endif //DEBUG_LOGIC

namespace modules {
namespace selector {

Selector selector;

void Selector::PrepareMoveToPlannedSlot() {
    mm::motion.PlanMoveTo<mm::Selector>(SlotPosition(plannedSlot), mm::unitToAxisUnit<mm::S_speed_t>(config::selectorFeedrate));
#ifdef DEBUG_LOGIC
    char str[30];
    sprintf_P(str, PSTR("Prepare Move Selector slot %d\n"), plannedSlot);
    hu::usart1.puts(str);
#endif //DEBUG_LOGIC
}

void Selector::PlanHomingMove() {
    mm::motion.PlanMove<mm::Selector>(mm::unitToAxisUnit<mm::S_pos_t>(config::selectorLimits.lenght * 2), mm::unitToAxisUnit<mm::S_speed_t>(config::selectorFeedrate));
#ifdef DEBUG_LOGIC
    hu::usart1.puts("Plan Homing Selector\n");
#endif //DEBUG_LOGIC
}

Selector::OperationResult Selector::MoveToSlot(uint8_t slot) {
    if (state == Moving) {
#ifdef DEBUG_LOGIC
        hu::usart1.puts("Moving --> Selector refused\n");
#endif //DEBUG_LOGIC
        return OperationResult::Refused;
    }
    plannedSlot = slot;

    if (currentSlot == slot) {
#ifdef DEBUG_LOGIC
        hu::usart1.puts("Moving Selector\n");
#endif //DEBUG_LOGIC
        return OperationResult::Accepted;
    }
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
#ifdef DEBUG_LOGIC
        //hu::usart1.puts("Moving Selector\n");
#endif //DEBUG_LOGIC
        return false;
    case Homing:
#ifdef DEBUG_LOGIC
        hu::usart1.puts("Homing Selector\n");
#endif //DEBUG_LOGIC
        PerformHome(mm::Selector);
        return false;
    case Ready:
#ifdef DEBUG_LOGIC
        //hu::usart1.puts("Selector Ready\n");
#endif //DEBUG_LOGIC
        currentSlot = plannedSlot;
        mm::motion.Disable(mm::Selector); // turn off selector motor's power every time
        return true;
    case Failed:
#ifdef DEBUG_LOGIC
        hu::usart1.puts("Selector Failed\n");
#endif //DEBUG_LOGIC
    default:
        return true;
    }
}

} // namespace selector
} // namespace modules
