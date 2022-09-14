/// @file selector.cpp
#include "selector.h"
#include "buttons.h"
#include "finda.h"
#include "leds.h"
#include "motion.h"
#include "permanent_storage.h"
#include "../debug.h"
#include "globals.h"
#include "idler.h" // @@TODO this is not nice - introduces dependency between the idler and selector - presumably electrical reasons :(

namespace modules {
namespace selector {

Selector selector;

void Selector::PrepareMoveToPlannedSlot() {
    mm::motion.PlanMoveTo<mm::Selector>(SlotPosition(plannedSlot), mm::unitToAxisUnit<mm::S_speed_t>(mg::globals.SelectorFeedrate_mm_s()));
    dbg_logic_fP(PSTR("Prepare Move Selector slot %d"), plannedSlot);
}

void Selector::PlanHomingMoveForward() {
    state = PlannedHome;
    dbg_logic_P(PSTR("Plan Homing Selector Forward"));
}

void Selector::PlanHomingMoveBack() {
    // we expect that we are at the front end of the axis, set the expected axis' position
    mm::motion.SetPosition(mm::Selector, 0);
    axisStart = mm::axisUnitToTruncatedUnit<config::U_mm>(mm::motion.CurPosition<mm::Selector>());
    mm::motion.PlanMove<mm::Selector>(mm::unitToAxisUnit<mm::S_pos_t>(config::selectorLimits.lenght * 2),
        mm::unitToAxisUnit<mm::S_speed_t>(config::selectorHomingFeedrate));
    dbg_logic_P(PSTR("Plan Homing Selector Back"));
}

bool Selector::FinishHomingAndPlanMoveToParkPos() {
    // check the axis' length
    int32_t axisEnd = mm::axisUnitToTruncatedUnit<config::U_mm>(mm::motion.CurPosition<mm::Selector>());
    if (abs(axisEnd - axisStart) < (config::selectorLimits.lenght.v - 3)) { //@@TODO is 3mm ok?
        return false; // we couldn't home correctly, we cannot set the Selector's position
    }

    mm::motion.SetPosition(mm::Selector, mm::unitToSteps<mm::S_pos_t>(config::selectorLimits.lenght));
    currentSlot = -1;

    // finish whatever has been planned before homing
    if (plannedSlot > config::toolCount) {
        plannedSlot = IdleSlotIndex();
    }
    InitMovement();
    return true;
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

    // if we are homing right now, just record the desired planned slot and return Accepted
    if (state == HomeBack) {
        return OperationResult::Accepted;
    }

    // This seems to cause all kinds of strange issues like errorneously ejected filaments from the wrong slot (because selector refused to move).
    // Moreover, FINDA looks very unreliable over time and occasionally causes spurious triggers.
    // But relying on a higher-level status (filament load state) could solve the issue,
    // because it is only updated at specific spots sometimes also according to FINDA status.
    //if (mf::finda.Pressed()) {
    if( mg::globals.FilamentLoaded() > mg::FilamentLoadState::AtPulley ){
        // @@TODO not sure why (if) this happens, but anyway - we must not move the selector if we think that we have filament in the selector.
        // That includes the CutFilament operation as well
        return OperationResult::Refused;
    }

    // coordinates invalid, first home, then engage
    if (!homingValid && mg::globals.FilamentLoaded() < mg::FilamentLoadState::InSelector) {
        PlanHome();
        return OperationResult::Accepted;
    }

    // already at the right slot
    if (currentSlot == slot) {
        dbg_logic_P(PSTR("Moving Selector"));
        return OperationResult::Accepted;
    }

    // do the move
    return InitMovement();
}

bool Selector::Step() {
    switch (state) {
    case Moving:
        PerformMove();
        //dbg_logic_P(PSTR("Moving Selector"));
        return false;
    case PlannedHome:
        // A testing workaround for presumed electrical reasons why the Idler and Selector cannot perform reliable homing together.
        // Let's wait for the Idler to finish homing before homing the selector.
        // This will surely break the unit tests, but that's not the point at this stage.
        if (mi::idler.HomingValid()) {
            // idler is ok, we can start homing the selector
            state = HomeForward;
            mm::motion.PlanMove<mm::Selector>(mm::unitToAxisUnit<mm::S_pos_t>(-config::selectorLimits.lenght * 2),
                mm::unitToAxisUnit<mm::S_speed_t>(config::selectorHomingFeedrate));
        }
        return false;
    case HomeForward:
        dbg_logic_P(PSTR("Homing Selector Forward"));
        PerformHomeForward();
        return false;
    case HomeBack:
        dbg_logic_P(PSTR("Homing Selector"));
        PerformHomeBack();
        return false;
    case Ready:
        if (!homingValid && mg::globals.FilamentLoaded() < mg::InSelector && (!mf::finda.Pressed())) {
            PlanHome();
            return false;
        }
        return true;
    case TMCFailed:
        dbg_logic_P(PSTR("Selector Failed"));
    default:
        return true;
    }
}

void Selector::Init() {
    if (mg::globals.FilamentLoaded() < mg::FilamentLoadState::InSelector && (!mf::finda.Pressed())) {
        // home the Selector only in case we don't have filament loaded (or at least we think we don't)
        PlanHome();
    } else {
        // otherwise set selector's position according to know slot positions (and pretend it is correct)
        mm::motion.SetPosition(mm::Selector, SlotPosition(mg::globals.ActiveSlot()).v);
        InvalidateHoming(); // and plan homing sequence ASAP
    }
}

} // namespace selector
} // namespace modules
