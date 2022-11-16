/// @file tool_change.cpp
#include "tool_change.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/fsensor.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../modules/pulley.h"
#include "../modules/selector.h"
#include "../modules/user_input.h"
#include "../debug.h"

namespace logic {

ToolChange toolChange;

bool ToolChange::Reset(uint8_t param) {
    if (!CheckToolIndex(param)) {
        return false;
    }

    if (param == mg::globals.ActiveSlot() && (mg::globals.FilamentLoaded() == mg::FilamentLoadState::InNozzle)) {
        // we are already at the correct slot and the filament is loaded in the nozzle - nothing to do
        dbg_logic_P(PSTR("we are already at the correct slot and the filament is loaded - nothing to do\n"));
        return true;
    }

    // @@TODO establish printer in charge of UI processing for the ToolChange command only.
    // We'll see how that works and then probably we'll introduce some kind of protocol settings to switch UI handling.
    mui::userInput.SetPrinterInCharge(true);

    // we are either already at the correct slot, just the filament is not loaded - load the filament directly
    // or we are standing at another slot ...
    plannedSlot = param;

    if (mg::globals.FilamentLoaded() >= mg::FilamentLoadState::InSelector) {
        dbg_logic_P(PSTR("Filament is loaded --> unload"));
        state = ProgressCode::UnloadingFilament;
        unloadAlreadyFinished = false;
        unl.Reset(mg::globals.ActiveSlot());
    } else {
        unloadAlreadyFinished = true;
        if (feed.Reset(true, false)) {
            state = ProgressCode::FeedingToFinda;
            error = ErrorCode::RUNNING;
            //        mg::globals.SetFilamentLoaded(plannedSlot, mg::FilamentLoadState::InSelector); // this is set in feed @@TODO
            dbg_logic_P(PSTR("Filament is not loaded --> load"));
        } else {
            // selector refused to move - FINDA problem suspected
            GoToErrDisengagingIdler(ErrorCode::FINDA_FLICKERS);
        }
    }
    return true;
}

void logic::ToolChange::GoToFeedingToBondtech() {
    ml::leds.SetPairButOffOthers(mg::globals.ActiveSlot(), ml::blink0, ml::off);
    james.Reset(3);
    state = ProgressCode::FeedingToBondtech;
    error = ErrorCode::RUNNING;
}

void logic::ToolChange::ToolChangeFinishedCorrectly() {
    ml::leds.SetPairButOffOthers(mg::globals.ActiveSlot(), ml::on, ml::off);
    mui::userInput.SetPrinterInCharge(false);
    FinishedOK();
}

void logic::ToolChange::GoToFeedingToFinda() {
    state = ProgressCode::FeedingToFinda;
    error = ErrorCode::RUNNING;
    mg::globals.SetFilamentLoaded(plannedSlot, mg::FilamentLoadState::AtPulley);
    if (!feed.Reset(true, false)) {
        GoToErrDisengagingIdler(ErrorCode::FINDA_FLICKERS);
    }
}

bool ToolChange::StepInner() {
    switch (state) {
    case ProgressCode::UnloadingFilament:
        if (unl.StepInner()) {
            // unloading sequence finished - basically, no errors can occurr here
            // as UnloadFilament should handle all the possible error states on its own
            // There is no way the UnloadFilament to finish in an error state
            unloadAlreadyFinished = true;
            // But planning the next move can fail if Selector refuses moving to the next slot
            // - that scenario is handled inside GoToFeedingToFinda
            GoToFeedingToFinda();
        }
        break;
    case ProgressCode::FeedingToFinda:
        if (feed.Step()) {
            if (feed.State() == FeedToFinda::Failed) {
                GoToErrDisengagingIdler(ErrorCode::FINDA_DIDNT_SWITCH_ON); // signal loading error
            } else {
                GoToFeedingToBondtech();
            }
        }
        break;
    case ProgressCode::FeedingToBondtech:
        if (james.Step()) {
            switch (james.State()) {
            case FeedToBondtech::Failed:
                GoToErrDisengagingIdler(ErrorCode::FSENSOR_DIDNT_SWITCH_ON); // signal loading error
                break;
            case FeedToBondtech::FSensorTooEarly:
                GoToErrDisengagingIdler(ErrorCode::FSENSOR_TOO_EARLY); // signal loading error
                break;
            default:
                ToolChangeFinishedCorrectly();
            }
        }
        break;
    case ProgressCode::OK:
        return true;

    case ProgressCode::ERRDisengagingIdler:
        ErrDisengagingIdler();
        return false;
    case ProgressCode::ERRWaitingForUser: {
        // waiting for user buttons and/or a command from the printer
        mui::Event ev = mui::userInput.ConsumeEvent();
        switch (ev) {
        case mui::Event::Left: // try to manually load just a tiny bit - help the filament with the pulley
            GoToErrEngagingIdler();
            break;
        case mui::Event::Middle: // try again the whole sequence
            // It looks like we don't have to reset the whole state machine but jump straight into the feeding phase.
            // The reasons are multiple:
            // - If an error happens during the unload phase, it is handled separately in the UnloadFilament state machine
            // - If an error happens during the feeding phase, the unload has been already successfully completed.
            //   And when restarted from the very beginning, the ToolChange does the last retract sequence from the UnloadFilament phase
            //   -> that is not healthy, because the filament gets pushed away from the Pulley and causes another error.
            // However - if we run into "FSensor didn't trigger", the situation is exactly opposite - it is beneficial
            // to unload the filament and try the whole sequence again
            // Therefore we only switch to FeedingToFinda if FINDA is not pressed (we suppose the filament is unloaded completely)
            //
            // MMU-191: if FSENSOR_DIDNT_SWITCH_ON was caused by misaligned Idler,
            // rehoming it may fix the issue when auto retrying -> no user intervention
            // So first invalidate homing flags as the user may have moved the Idler or Selector accidentally.
            //
            // Beware: we may run into issues when FINDA or FSensor do not work correctly. Selector may rely on the presumed filament position and actually cut it accidentally when trying to rehome.
            // It is yet to be seen if something like this can actually happen.
            InvalidateHoming();
            // This is a tricky part in case FINDA is flickering
            // If we already managed to finish the unload, we must assume finda should be NOT pressed.
            // If it is still pressed
            //  -> FINDA is flickering/badly tuned
            //  -> unreliable and the user didn't fix the issue
            //  -> we cannot do anything else but request the user to fix FINDA
            if (mf::finda.Pressed() && (!unloadAlreadyFinished)) {
                Reset(mg::globals.ActiveSlot());
            } else {
                GoToFeedingToFinda();
            }
            break;
        case mui::Event::Right: // problem resolved - the user pushed the fillament by hand?
            // we should check the state of all the sensors and either report another error or confirm the correct state

            // First invalidate homing flags as the user may have moved the Idler or Selector accidentally
            InvalidateHoming();
            if (!mf::finda.Pressed()) {
                // FINDA is still NOT pressed - that smells bad
                error = ErrorCode::FINDA_DIDNT_SWITCH_ON;
                state = ProgressCode::ERRWaitingForUser; // stand still
            } else if (!mfs::fsensor.Pressed()) {
                // printer's filament sensor is still NOT pressed - that smells bad
                mg::globals.SetFilamentLoaded(plannedSlot, mg::FilamentLoadState::InSelector); // only assume the filament is in selector
                error = ErrorCode::FSENSOR_DIDNT_SWITCH_ON;
                state = ProgressCode::ERRWaitingForUser; // stand still - we may even try loading the filament into the nozzle
            } else {
                // all sensors are ok, we assume the user pushed the filament into the nozzle
                mg::globals.SetFilamentLoaded(plannedSlot, mg::FilamentLoadState::InNozzle);
                ToolChangeFinishedCorrectly();
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
            mpu::pulley.PlanMove(config::pulleyHelperMove, config::pulleySlowFeedrate);
        }
        return false;
    case ProgressCode::ERRHelpingFilament:
        // @@TODO helping filament needs improvement - the filament should try to move forward as long as the button is pressed
        if (mf::finda.Pressed()) {
            // the help was enough to press the FINDA, we are ok, continue normally
            GoToFeedingToBondtech();
        } else if (mfs::fsensor.Pressed()) {
            // the help was enough to press the filament sensor, we are ok, continue normally
            GoToFeedingToBondtech();
            // Beware, when the fsensor triggers, we still need to push the filament into the nozzle/gears
            // which requires restarting James from its last stage
            james.GoToPushToNozzle();
        } else if (mm::motion.QueueEmpty()) {
            // helped a bit, but FINDA/Fsensor didn't trigger, return to the main error state
            GoToErrDisengagingIdler(ErrorCode::FSENSOR_DIDNT_SWITCH_ON);
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
    case ProgressCode::FeedingToBondtech:
        // only process the important states
        switch (james.State()) {
        case FeedToBondtech::PushingFilamentToFSensor:
            return ProgressCode::FeedingToFSensor;
        case FeedToBondtech::PushingFilamentIntoNozzle:
            return ProgressCode::FeedingToNozzle;
        case FeedToBondtech::DisengagingIdler:
            return ProgressCode::DisengagingIdler;
        }
        [[fallthrough]]; // everything else is reported as is
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
