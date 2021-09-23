#include "idler.h"
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
namespace idler {

Idler idler;

void Idler::PrepareMoveToPlannedSlot() {
    mm::motion.PlanMoveTo<mm::Idler>(SlotPosition(plannedSlot), mm::unitToAxisUnit<mm::I_speed_t>(config::idlerFeedrate));
#ifdef DEBUG_LOGIC
    char str[30];
    sprintf_P(str, PSTR("Prepare Move Idler slot %d\n"), plannedSlot);
    hu::usart1.puts(str);
#endif //DEBUG_LOGIC
}

void Idler::PlanHomingMove() {
    mm::motion.PlanMove<mm::Idler>(mm::unitToAxisUnit<mm::I_pos_t>(-config::idlerLimits.lenght * 2), mm::unitToAxisUnit<mm::I_speed_t>(config::idlerFeedrate));
#ifdef DEBUG_LOGIC
    hu::usart1.puts("Plan Homing Idler\n");
#endif //DEBUG_LOGIC
}

Idler::OperationResult Idler::Disengage() {
    if (state == Moving) {
#ifdef DEBUG_LOGIC
        hu::usart1.puts("Moving --> Disengage refused\n");
#endif //DEBUG_LOGIC
        return OperationResult::Refused;
    }
    plannedSlot = IdleSlotIndex();
    plannedEngage = false;

    if (!Engaged()) {
#ifdef DEBUG_LOGIC
        hu::usart1.puts("Disengage Idler\n");
#endif //DEBUG_LOGIC
        return OperationResult::Accepted;
    }
    return InitMovement(mm::Idler);
}

Idler::OperationResult Idler::Engage(uint8_t slot) {
    if (state == Moving) {
#ifdef DEBUG_LOGIC
        hu::usart1.puts("Moving --> Engage refused\n");
#endif //DEBUG_LOGIC
        return OperationResult::Refused;
    }

    plannedSlot = slot;
    plannedEngage = true;

    if (Engaged()) {
#ifdef DEBUG_LOGIC
        hu::usart1.puts("Engage Idler\n");
#endif //DEBUG_LOGIC
        return OperationResult::Accepted;
    }

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
#ifdef DEBUG_LOGIC
        //hu::usart1.puts("Moving Idler\n");
#endif //DEBUG_LOGIC
        PerformMove(mm::Idler);
        return false;
    case Homing:
#ifdef DEBUG_LOGIC
        hu::usart1.puts("Homing Idler\n");
#endif //DEBUG_LOGIC
        PerformHome(mm::Idler);
        return false;
    case Ready:
#ifdef DEBUG_LOGIC
        //hu::usart1.puts("Idler Ready\n");
#endif //DEBUG_LOGIC
        currentlyEngaged = plannedEngage;
        currentSlot = plannedSlot;

        if (!Engaged()) // turn off power into the Idler motor when disengaged (no force necessary)
            mm::motion.Disable(mm::Idler);

        return true;
    case Failed:
#ifdef DEBUG_LOGIC
        hu::usart1.puts("Idler Failed\n");
#endif //DEBUG_LOGIC
    default:
        return true;
    }
}

} // namespace idler
} // namespace modules
