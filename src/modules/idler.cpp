#include "idler.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"
#include "../debug.h"

namespace modules {
namespace idler {

Idler idler;

void Idler::PrepareMoveToPlannedSlot() {
    mm::motion.PlanMoveTo<mm::Idler>(SlotPosition(plannedSlot), mm::unitToAxisUnit<mm::I_speed_t>(config::idlerFeedrate));
    dbg_logic_sprintf_P(PSTR("Prepare Move Idler slot %d"), plannedSlot);
}

void Idler::PlanHomingMove() {
    mm::motion.PlanMove<mm::Idler>(mm::unitToAxisUnit<mm::I_pos_t>(-config::idlerLimits.lenght * 2), mm::unitToAxisUnit<mm::I_speed_t>(config::idlerFeedrate));
    dbg_logic_P(PSTR("Plan Homing Idler"));
}

Idler::OperationResult Idler::Disengage() {
    if (state == Moving) {
        dbg_logic_P(PSTR("Moving --> Disengage refused"));
        return OperationResult::Refused;
    }
    plannedSlot = IdleSlotIndex();
    plannedEngage = false;

    if (!Engaged()) {
        dbg_logic_P(PSTR("Disengage Idler"));
        return OperationResult::Accepted;
    }
    return InitMovement(mm::Idler);
}

Idler::OperationResult Idler::Engage(uint8_t slot) {
    if (state == Moving) {
        dbg_logic_P(PSTR("Moving --> Engage refused"));
        return OperationResult::Refused;
    }

    plannedSlot = slot;
    plannedEngage = true;

    if (Engaged()) {
        dbg_logic_P(PSTR("Engage Idler"));
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
        // dbg_logic_P(PSTR("Moving Idler"));
        PerformMove(mm::Idler);
        return false;
    case Homing:
        dbg_logic_P(PSTR("Homing Idler"));
        PerformHome(mm::Idler);
        return false;
    case Ready:
        // dbg_logic_P(PSTR("Idler Ready"));
        currentlyEngaged = plannedEngage;
        currentSlot = plannedSlot;

        if (!Engaged()) // turn off power into the Idler motor when disengaged (no force necessary)
            mm::motion.Disable(mm::Idler);

        return true;
    case Failed:
        dbg_logic_P(PSTR("Idler Failed"));
    default:
        return true;
    }
}

} // namespace idler
} // namespace modules
