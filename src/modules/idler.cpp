/// @file idler.cpp
#include "idler.h"
#include "buttons.h"
#include "globals.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"
#include "../debug.h"

namespace modules {
namespace idler {

Idler idler;

void Idler::PrepareMoveToPlannedSlot() {
    mm::motion.PlanMoveTo<mm::Idler>(SlotPosition(plannedSlot), mm::unitToAxisUnit<mm::I_speed_t>(config::idlerFeedrate));
    dbg_logic_fP(PSTR("Prepare Move Idler slot %d"), plannedSlot);
}

void Idler::PlanHomingMove() {
    mm::motion.PlanMove<mm::Idler>(mm::unitToAxisUnit<mm::I_pos_t>(-config::idlerLimits.lenght * 2), mm::unitToAxisUnit<mm::I_speed_t>(config::idlerFeedrate));
    dbg_logic_P(PSTR("Plan Homing Idler"));
}

void Idler::FinishHomingAndPlanMoveToParkPos() {
    mm::motion.SetPosition(mm::Idler, 0);

    // finish whatever has been planned before homing
    if (!plannedEngage) {
        plannedSlot = IdleSlotIndex();
    }
    InitMovement();
}

void Idler::FinishMove() {
    currentlyEngaged = plannedEngage;
    if (!Engaged()) // turn off power into the Idler motor when disengaged (no force necessary)
        mm::motion.Disable(mm::Idler);
}

Idler::OperationResult Idler::Disengage() {
    if (state == Moving) {
        dbg_logic_P(PSTR("Moving --> Disengage refused"));
        return OperationResult::Refused;
    }
    plannedSlot = IdleSlotIndex();
    plannedEngage = false;

    // coordinates invalid, first home, then disengage
    if (!homingValid) {
        PerformHome();
        return OperationResult::Accepted;
    }

    // already disengaged
    if (!Engaged()) {
        dbg_logic_P(PSTR("Idler Disengaged"));
        return OperationResult::Accepted;
    }

    // disengaging
    return InitMovement();
}

Idler::OperationResult Idler::Engage(uint8_t slot) {
    if (state == Moving) {
        dbg_logic_P(PSTR("Moving --> Engage refused"));
        return OperationResult::Refused;
    }

    plannedSlot = slot;
    plannedEngage = true;

    // if we are homing right now, just record the desired planned slot and return Accepted
    if (state == Homing) {
        return OperationResult::Accepted;
    }

    // coordinates invalid, first home, then engage
    // The MMU FW only decides to engage the Idler when it is supposed to do something and not while it is idle
    // so rebooting the MMU while the printer is printing (and thus holding the filament by the moving Idler)
    // should not be an issue
    if (!homingValid) {
        PlanHome();
        return OperationResult::Accepted;
    }

    // already engaged
    if (Engaged()) {
        dbg_logic_P(PSTR("Idler Engaged"));
        return OperationResult::Accepted;
    }

    // engaging
    return InitMovement();
}

bool Idler::Step() {
    switch (state) {
    case Moving:
        // dbg_logic_P(PSTR("Moving Idler"));
        PerformMove();
        return false;
    case Homing:
        dbg_logic_P(PSTR("Homing Idler"));
        PerformHome();
        return false;
    case Ready:
        if (!homingValid && mg::globals.FilamentLoaded() < mg::InFSensor) {
            PlanHome();
            return false;
        }
        return true;
    case TMCFailed:
        dbg_logic_P(PSTR("Idler Failed"));
    default:
        return true;
    }
}

void Idler::Init() {
    if (mg::globals.FilamentLoaded() < mg::InFSensor) {
        // home the Idler only in case we don't have filament loaded in the printer (or at least we think we don't)
        PlanHome();
    } else {
        // otherwise assume the Idler is at its idle position (that's where it usually is)
        mm::motion.SetPosition(mm::Idler, SlotPosition(IdleSlotIndex()).v);
        InvalidateHoming(); // and plan homing sequence ASAP
    }
}

} // namespace idler
} // namespace modules
