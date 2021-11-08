/// @file load_filament.cpp
#include "load_filament.h"
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

LoadFilament loadFilament;

void LoadFilament::Reset(uint8_t param) {
    if (!CheckToolIndex(param)) {
        return;
    }
    dbg_logic_P(PSTR("Load Filament"));
    mg::globals.SetFilamentLoaded(param, mg::FilamentLoadState::AtPulley); // still at pulley, haven't moved yet
    Reset2();
}

void logic::LoadFilament::Reset2() {
    state = ProgressCode::FeedingToFinda;
    error = ErrorCode::RUNNING;
    feed.Reset(true);
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::blink0);
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
}

void logic::LoadFilament::GoToRetractingFromFinda() {
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::blink0);
    state = ProgressCode::RetractingFromFinda;
    error = ErrorCode::RUNNING;
    retract.Reset();
}

bool LoadFilament::StepInner() {
    switch (state) {
    case ProgressCode::FeedingToFinda:
        if (feed.Step()) {
            if (feed.State() == FeedToFinda::Failed) {
                // @@TODO - try to repeat 6x - push/pull sequence - probably something to put into feed_to_finda as an option
                state = ProgressCode::ERRDisengagingIdler;
                error = ErrorCode::FINDA_DIDNT_SWITCH_ON;
                mi::idler.Disengage();
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::blink0); // signal loading error
            } else {
                state = ProgressCode::RetractingFromFinda;
                retract.Reset();
            }
        }
        break;
    case ProgressCode::RetractingFromFinda:
        if (retract.Step()) {
            if (retract.State() == RetractFromFinda::Failed) {
                state = ProgressCode::ERRDisengagingIdler;
                error = ErrorCode::FINDA_DIDNT_SWITCH_OFF;
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::blink0); // signal loading error
            } else {
                state = ProgressCode::DisengagingIdler;
            }
            mi::idler.Disengage(); // disengage in both cases
        }
        break;
    case ProgressCode::DisengagingIdler:
        if (!mi::idler.Engaged()) {
            state = ProgressCode::OK;
            error = ErrorCode::OK;
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
            mm::motion.Disable(mm::Pulley);
        }
        break;
    case ProgressCode::OK:
        return true;
    case ProgressCode::ERRDisengagingIdler: // couldn't load to FINDA
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
            // however it depends on the state of FINDA - if it is on, we must perform unload first
            if (!mf::finda.Pressed()) {
                Reset2();
            } else {
                GoToRetractingFromFinda();
            }
            break;
        case mui::Event::Right: // problem resolved - the user pushed the fillament by hand?
            // we should check the state of all the sensors and either report another error or confirm the correct state
            if (!mf::finda.Pressed()) {
                // FINDA is still NOT pressed - that smells bad
                error = ErrorCode::FINDA_DIDNT_SWITCH_ON;
                state = ProgressCode::ERRWaitingForUser; // stand still
            } else {
                // all sensors are ok - pull the filament back
                GoToRetractingFromFinda();
            }
            break;
        default: // no event, continue waiting for user input
            break;
        }
        return false;
    }
    case ProgressCode::ERREngagingIdler:
        if (mi::idler.Engaged()) {
            state = ProgressCode::ERRHelpingFilament;
            mm::motion.PlanMove<mm::Pulley>(config::pulleyHelperMove, config::pulleySlowFeedrate);
        }
        return false;
    case ProgressCode::ERRHelpingFilament:
        if (mf::finda.Pressed()) {
            // the help was enough to press the FINDA, we are ok, continue normally
            GoToRetractingFromFinda();
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

} // namespace logic
