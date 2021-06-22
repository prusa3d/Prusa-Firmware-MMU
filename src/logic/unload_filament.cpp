#include "unload_filament.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"

namespace logic {

UnloadFilament unloadFilament;

namespace mb = modules::buttons;
namespace mm = modules::motion;
namespace mi = modules::idler;
namespace ml = modules::leds;
namespace mg = modules::globals;

void UnloadFilament::Reset(uint8_t /*param*/) {
    // unloads filament from extruder - filament is above Bondtech gears
    mm::motion.InitAxis(mm::Pulley);
    state = ProgressCode::UnloadingToFinda;
    error = ErrorCode::OK;
    unl.Reset(maxRetries);
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::blink0);
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
}

bool UnloadFilament::Step() {
    switch (state) {
    // state 1 engage idler - will be done by the Unload to FINDA state machine
    case ProgressCode::UnloadingToFinda: // state 2 rotate pulley as long as the FINDA is on
        if (unl.Step()) {
            if (unl.State() == UnloadToFinda::Failed) {
                // couldn't unload to FINDA, report error and wait for user to resolve it
                state = ProgressCode::ERR1DisengagingIdler;
                error = ErrorCode::FINDA_DIDNT_TRIGGER;
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::blink0);
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
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
            mm::motion.DisableAxis(mm::Pulley);
            mg::globals.SetFilamentLoaded(false); // filament unloaded
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::on);
        }
        return false;
    case ProgressCode::ERR1DisengagingIdler: // couldn't unload to FINDA
        if (!mi::idler.Engaged()) {
            state = ProgressCode::ERR1WaitingForUser;
        }
        return false;
    case ProgressCode::ERR1WaitingForUser: {
        // waiting for user buttons and/or a command from the printer
        bool help = mb::buttons.ButtonPressed(mb::Left) /*|| command_help()*/;
        bool tryAgain = mb::buttons.ButtonPressed(mb::Middle) /*|| command_tryAgain()*/;
        bool userResolved = mb::buttons.ButtonPressed(mb::Right) /*|| command_userResolved()*/;
        if (help) {
            // try to manually unload just a tiny bit - help the filament with the pulley
            //@@TODO
        } else if (tryAgain) {
            // try again the whole sequence
            Reset(0);
        } else if (userResolved) {
            // problem resolved - the user pulled the fillament by hand
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::on);
            //                mm::motion.PlanMove(mm::Pulley, 450, 5000); // @@TODO constants
            state = ProgressCode::AvoidingGrind;
        }
        return false;
    }
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
