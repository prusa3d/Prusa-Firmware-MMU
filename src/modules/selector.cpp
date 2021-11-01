/// @file selector.cpp
#include "selector.h"
#include "buttons.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"
#include "../debug.h"
#include "globals.h"

namespace modules {
namespace selector {

Selector selector;

void Selector::PrepareMoveToPlannedSlot() {
    mm::motion.PlanMoveTo<mm::Selector>(SlotPosition(plannedSlot), mm::unitToAxisUnit<mm::S_speed_t>(config::selectorFeedrate));
    dbg_logic_fP(PSTR("Prepare Move Selector slot %d"), plannedSlot);
}

void Selector::PlanHomingMove() {
    mm::motion.PlanMove<mm::Selector>(mm::unitToAxisUnit<mm::S_pos_t>(config::selectorLimits.lenght * 2), mm::unitToAxisUnit<mm::S_speed_t>(config::selectorFeedrate));
    dbg_logic_P(PSTR("Plan Homing Selector"));
}

void Selector::FinishHomingAndPlanMoveToParkPos() {
    mm::motion.SetPosition(mm::Selector, mm::unitToSteps<mm::S_pos_t>(config::selectorLimits.lenght));
    plannedSlot = IdleSlotIndex();
    InitMovement(mm::Selector);
}

void Selector::FinishMove() {
    mm::motion.Disable(mm::Selector); // turn off selector motor's power every time
}

Selector::OperationResult Selector::MoveToSlot(uint8_t slot) {
    if (state == Moving) {
        dbg_logic_P(PSTR("Moving --> Selector refused"));
        return OperationResult::Refused;
    }
    plannedSlot = slot;

    if (currentSlot == slot) {
        dbg_logic_P(PSTR("Moving Selector"));
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
        //dbg_logic_P(PSTR("Moving Selector"));
        return false;
    case Homing:
        dbg_logic_P(PSTR("Homing Selector"));
        PerformHome(mm::Selector);
        return false;
    case Ready:
        //dbg_logic_P(PSTR("Selector Ready"));
        return true;
    case Failed:
        dbg_logic_P(PSTR("Selector Failed"));
    default:
        return true;
    }
}

void Selector::Init() {
    if (mg::globals.FilamentLoaded() < mg::FilamentLoadState::InFSensor) {
        // home the Selector only in case we don't have filament loaded (or at least we think we don't)
        Home();
    } else {
        // otherwise set selector's position according to know slot positions (and pretend it is correct)
        mm::motion.SetPosition(mm::Selector, SlotPosition(mg::globals.ActiveSlot()).v);
    }
}

} // namespace selector
} // namespace modules
