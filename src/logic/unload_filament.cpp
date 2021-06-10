#include "unload_filament.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/idler.h"
#include "../modules/permanent_storage.h"

namespace logic {

UnloadFilament unloadFilament;

namespace mm = modules::motion;
namespace mi = modules::idler;

void UnloadFilament::Reset(uint8_t param) {
    // unloads filament from extruder - filament is above Bondtech gears
    mm::motion.InitAxis(mm::Pulley);
    state = ProgressCode::EngagingIdler;
    error = ErrorCode::OK;
    modules::idler::idler.Engage(0); //@@TODO
}

bool UnloadFilament::Step() {
    switch (state) {
    case ProgressCode::EngagingIdler: // state 1 engage idler
        if (mi::idler.Engaged()) { // if idler is in parked position un-park it get in contact with filament
            state = ProgressCode::UnloadingToFinda;
            unl.Reset();
        }
        return false;
    case ProgressCode::UnloadingToFinda: // state 2 rotate pulley as long as the FINDA is on
        if (unl.Step()) {
            if (unl.state == UnloadToFinda::Failed) {
                // couldn't unload to FINDA, report error and wait for user to resolve it
                state = ProgressCode::ERR1DisengagingIdler;
                //                    modules::leds::leds.SetMode(active_extruder, modules::leds::red, modules::leds::blink0);
            } else {
                state = ProgressCode::DisengagingIdler;
            }
            // in all cases disengage the idler
            mi::idler.Disengage();
        }
        return false;
    case ProgressCode::DisengagingIdler:
        if (!mi::idler.Engaged()) {
            state = ProgressCode::AvoidingGrind;
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
        }
        return false;
    case ProgressCode::ERR1DisengagingIdler: // couldn't unload to FINDA
        error = ErrorCode::UNLOAD_FINDA_DIDNT_TRIGGER;
        if (!mi::idler.Engaged()) {
            state = ProgressCode::ERR1WaitingForUser;
        }
        return false;
    case ProgressCode::ERR1WaitingForUser: {
        // waiting for user buttons and/or a command from the printer
        bool help = modules::buttons::buttons.ButtonPressed(modules::buttons::Left) /*|| command_help()*/;
        bool tryAgain = modules::buttons::buttons.ButtonPressed(modules::buttons::Middle) /*|| command_tryAgain()*/;
        bool userResolved = modules::buttons::buttons.ButtonPressed(modules::buttons::Right) /*|| command_userResolved()*/;
        if (help) {
            // try to manually unload just a tiny bit - help the filament with the pulley
            //@@TODO
        } else if (tryAgain) {
            // try again the whole sequence
            Reset(0); // @@TODO param
        } else if (userResolved) {
            // problem resolved - the user pulled the fillament by hand
            //                modules::leds::leds.SetMode(active_extruder, modules::leds::red, modules::leds::off);
            //                modules::leds::leds.SetMode(active_extruder, modules::leds::green, modules::leds::on);
            //                mm::motion.PlanMove(mm::Pulley, 450, 5000); // @@TODO constants
            state = ProgressCode::AvoidingGrind;
        }
        return false;
    }
    case ProgressCode::OK:
        //            isFilamentLoaded = false; // filament unloaded
        return true; // successfully finished
    }
    return false;
}

} // namespace logic
