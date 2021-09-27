#include "unload_filament.h"
#include "../modules/finda.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../modules/user_input.h"
#ifdef DEBUG_LOGIC
#include "../hal/usart.h"
#include <string.h>
#include <stdio.h>
#endif //DEBUG_LOGIC

namespace logic {

UnloadFilament unloadFilament;

void UnloadFilament::Reset(uint8_t /*param*/) {
    // unloads filament from extruder - filament is above Bondtech gears
    mm::motion.InitAxis(mm::Pulley);
    state = ProgressCode::UnloadingToFinda;
    error = ErrorCode::RUNNING;
    unl.Reset(maxRetries);
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
}

bool UnloadFilament::StepInner() {
    switch (state) {
    // state 1 engage idler - will be done by the Unload to FINDA state machine
    case ProgressCode::UnloadingToFinda: // state 2 rotate pulley as long as the FINDA is on
        if (unl.Step()) {
            if (unl.State() == UnloadToFinda::Failed) {
                // couldn't unload to FINDA, report error and wait for user to resolve it
                state = ProgressCode::ERRDisengagingIdler;
                error = ErrorCode::FINDA_DIDNT_SWITCH_OFF;
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::blink0);
            } else {
                state = ProgressCode::DisengagingIdler;
            }
            // in all cases disengage the idler
            mi::idler.Disengage();
        }
        return false;
    case ProgressCode::DisengagingIdler:
        if (!mi::idler.Engaged()) {
            state = ProgressCode::AvoidingGrind; // @@TODO what was this originally? Why are we supposed to move the pulley when the idler is not engaged?
            // may be the pulley was to move along with the idler?
            //                mm::motion.PlanMove(mm::Pulley, -100, 10); // @@TODO constants
        }
        return false;
    case ProgressCode::AvoidingGrind: // state 3 move a little bit so it is not a grinded hole in filament
        if (mm::motion.QueueEmpty()) {
            state = ProgressCode::FinishingMoves;
            mi::idler.Disengage();
            return true;
        }
        return false;
    case ProgressCode::FinishingMoves:
        if (mm::motion.QueueEmpty()) {
            state = ProgressCode::OK;
            error = ErrorCode::OK;
            mm::motion.Disable(mm::Pulley);
            mg::globals.SetFilamentLoaded(false); // filament unloaded
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
        }
        return false;
    case ProgressCode::ERRDisengagingIdler: // couldn't unload to FINDA
        ErrDisengagingIdler();
        return false;
    case ProgressCode::ERRWaitingForUser: {
        // waiting for user buttons and/or a command from the printer
        mui::Event ev = mui::userInput.ConsumeEvent();
        switch (ev) {
        case mui::Event::Left: // try to manually unload just a tiny bit - help the filament with the pulley
            state = ProgressCode::ERREngagingIdler;
            mi::idler.Engage(mg::globals.ActiveSlot());
            break;
        case mui::Event::Middle: // try again the whole sequence
            Reset(0); //@@TODO validate the reset parameter
            break;
        case mui::Event::Right: // problem resolved - the user pulled the fillament by hand
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::on);
            //                mm::motion.PlanMove(mm::Pulley, 450, 5000); // @@TODO constants
            state = ProgressCode::AvoidingGrind;
            break;
        default:
            break;
        }
        return false;
    }
    case ProgressCode::ERREngagingIdler:
        if (mi::idler.Engaged()) {
            state = ProgressCode::ERRHelpingFilament;
            mm::motion.PlanMove(mm::Pulley, 450, 5000);
        }
        return false;
    case ProgressCode::ERRHelpingFilament:
        if (!mf::finda.Pressed()) {
            // the help was enough to depress the FINDA, we are ok, continue normally
            state = ProgressCode::DisengagingIdler;
            error = ErrorCode::RUNNING;
        } else if (mm::motion.QueueEmpty()) {
            // helped a bit, but FINDA didn't trigger, return to the main error state
            state = ProgressCode::ERRDisengagingIdler;
        }
        return false;
    case ProgressCode::OK:
        return true; // successfully finished
    default: // we got into an unhandled state, better report it
        state = ProgressCode::ERRInternal;
        error = ErrorCode::INTERNAL;
        return true;
    }
    return false;
}

} // namespace logic
