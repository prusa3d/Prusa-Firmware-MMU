#include "tool_change.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../modules/selector.h"
#include "../modules/user_input.h"
#include "../debug.h"

namespace logic {

ToolChange toolChange;

void ToolChange::Reset(uint8_t param) {
    if (!CheckToolIndex(param)) {
        return;
    }

    if (param == mg::globals.ActiveSlot() && mg::globals.FilamentLoaded() == mg::FilamentLoadState::InNozzle) {
        // we are already at the correct slot and the filament is loaded - nothing to do
        dbg_logic_P(PSTR("we are already at the correct slot and the filament is loaded - nothing to do\n"));
        return;
    }

    // we are either already at the correct slot, just the filament is not loaded - load the filament directly
    // or we are standing at another slot ...
    plannedSlot = param;

    if (mg::globals.FilamentLoaded() >= mg::FilamentLoadState::InSelector) {
        dbg_logic_P(PSTR("Filament is loaded --> unload"));
        state = ProgressCode::UnloadingFilament;
        unl.Reset(mg::globals.ActiveSlot());
    } else {
        state = ProgressCode::FeedingToFinda;
        error = ErrorCode::RUNNING;
        dbg_logic_P(PSTR("Filament is not loaded --> load"));
        mg::globals.SetActiveSlot(plannedSlot);
        feed.Reset(true);
    }
}

bool ToolChange::StepInner() {
    switch (state) {
    case ProgressCode::UnloadingFilament:
        if (unl.StepInner()) {
            // unloading sequence finished - basically, no errors can occurr here
            // as UnloadFilament should handle all the possible error states on its own
            // There is no way the UnloadFilament to finish in an error state
            state = ProgressCode::FeedingToFinda;
            error = ErrorCode::RUNNING;
            feed.Reset(true);
        }
        break;
    case ProgressCode::FeedingToFinda:
        if (feed.Step()) {
            if (feed.State() == FeedToFinda::Failed) {
                state = ProgressCode::ERRDisengagingIdler;
                error = ErrorCode::FINDA_DIDNT_SWITCH_ON;
                mi::idler.Disengage();
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::blink0); // signal loading error
            } else {
                state = ProgressCode::FeedingToBondtech;
                james.Reset(3);
            }
        }
        break;
    case ProgressCode::FeedingToBondtech:
        if (james.Step()) {
            if (james.State() == FeedToBondtech::Failed) {
                state = ProgressCode::ERRDisengagingIdler;
                error = ErrorCode::FSENSOR_DIDNT_SWITCH_ON;
                mi::idler.Disengage();
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::blink0); // signal loading error
            } else {
                mg::globals.SetFilamentLoaded(mg::FilamentLoadState::InNozzle);
                state = ProgressCode::OK;
                error = ErrorCode::OK;
            }
        }
        break;
    case ProgressCode::OK:
        return true;

    // @@TODO error handling definitely needs unifying with the LoadFilament state machine
    case ProgressCode::ERRDisengagingIdler:
        ErrDisengagingIdler();
        return false;
    case ProgressCode::ERRWaitingForUser: {
        // waiting for user buttons and/or a command from the printer
        mui::Event ev = mui::userInput.ConsumeEvent();
        switch (ev) {
        case mui::Event::Left: // try to manually load just a tiny bit - help the filament with the pulley
            state = ProgressCode::ERREngagingIdler;
            mi::idler.Engage(mg::globals.ActiveSlot());
            break;
        case mui::Event::Middle: // try again the whole sequence
            Reset(mg::globals.ActiveSlot());
            break;
        case mui::Event::Right: // problem resolved - the user pushed the fillament by hand?
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::on);
            //                mm::motion.PlanMove(mm::Pulley, 450, 5000); // @@TODO constants
            state = ProgressCode::AvoidingGrind;
            break;
        default: // no event, continue waiting for user input
            break;
        }
        return false;
    }
    case ProgressCode::ERREngagingIdler:
        if (mi::idler.Engaged()) {
            state = ProgressCode::ERRHelpingFilament;
            mm::motion.PlanMove(mm::Pulley, 450, 5000); //@@TODO constants
        }
        return false;
    case ProgressCode::ERRHelpingFilament:
        if (mf::finda.Pressed()) {
            // the help was enough to press the FINDA, we are ok, continue normally
            state = ProgressCode::FeedingToBondtech;
            error = ErrorCode::RUNNING;
        } else if (mm::motion.QueueEmpty()) {
            // helped a bit, but FINDA didn't trigger, return to the main error state
            state = ProgressCode::ERRDisengagingIdler;
        }
        return false;
    default: // we got into an unhandled state, better report it
        state = ProgressCode::ERRInternal;
        error = ErrorCode::INTERNAL;
        return true;
    }
    return false;
}

ProgressCode ToolChange::State() const {
    switch (state) {
    case ProgressCode::UnloadingFilament:
        return unl.State(); // report sub-automaton states properly
    default:
        return state;
    }
}

ErrorCode ToolChange::Error() const {
    switch (state) {
    case ProgressCode::UnloadingFilament: {
        ErrorCode ec = unl.Error(); // report sub-automaton errors properly, only filter out OK and replace them with RUNNING
        return ec == ErrorCode::OK ? ErrorCode::RUNNING : ec;
    }
    default:
        return error;
    }
}

} // namespace logic
