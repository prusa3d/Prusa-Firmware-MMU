/// @file unload_to_finda.cpp
#include "unload_to_finda.h"
#include "../modules/finda.h"
#include "../modules/fsensor.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../modules/pulley.h"

namespace logic {

void UnloadToFinda::Reset(uint8_t maxTries) {
    this->maxTries = maxTries;
    // check the inital state of FINDA and plan the moves
    if (!mf::finda.Pressed()) {
        state = OK; // FINDA is already off, we assume the fillament is not there, i.e. already unloaded
    } else {
        // FINDA is sensing the filament, plan moves to unload it
        mi::idler.Engage(mg::globals.ActiveSlot());
        if (mg::globals.FilamentLoaded() >= mg::FilamentLoadState::InSelector) {
            state = EngagingIdler;
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::blink0);
        } else {
            state = FailedFINDA;
        }
    }
}

bool UnloadToFinda::Step() {
    switch (state) {
    case EngagingIdler:
        if (mi::idler.Engaged()) {
            state = UnloadingFromFSensor;
            mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::FilamentLoadState::InSelector);
            unloadStart_mm = mpu::pulley.CurrentPosition_mm();
            // plan both moves to keep the unload smooth
            mpu::pulley.InitAxis();
            mpu::pulley.PlanMove(-mg::globals.FSensorUnloadCheck_mm(), mg::globals.PulleySlowFeedrate_mm_s());
            mpu::pulley.PlanMove(-config::defaultBowdenLength - config::feedToFinda - config::filamentMinLoadedToMMU // standard lenght where FINDA is expected to trigger
                    + mg::globals.FSensorUnloadCheck_mm(), // but subtract the slow unload phase distance
                mg::globals.PulleyUnloadFeedrate_mm_s());
        }
        return false;
    case UnloadingFromFSensor: {
        int32_t mpucp = mpu::pulley.CurrentPosition_mm();
        if ((abs(unloadStart_mm - mpucp) > mm::truncatedUnit(mg::globals.FSensorUnloadCheck_mm()))) {
            // passed the slow unload distance, check fsensor
            if (mfs::fsensor.Pressed()) {
                // fsensor didn't trigger within the first fsensorUnloadCheckDistance mm -> stop pulling, something failed, report an error
                // This scenario should not be tried again - repeating it may cause more damage to filament + potentially more collateral damage
                state = FailedFSensor;
                mm::motion.AbortPlannedMoves(); // stop rotating the pulley
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
                return false; // avoid falling into WaitingForFINDA
            } else {
                // fsensor turned correctly off, seamlessly transfer to fast unloading -> waiting for FINDA
                // the move has already been planned when idler engaged
                state = WaitingForFINDA;
                // in this case we can safely fall into the WaitingForFINDA state as well
            }
        }
    }
        [[fallthrough]]; // while UnloadingFromFSensor also check for FINDA switch off
        // if that happens, filament has either broken (its tip reached finda before fsensor switched off)
        // or there is some other problem.
    case WaitingForFINDA: {
        if (!mf::finda.Pressed()) {
            // detected end of filament
            state = OK;
            mm::motion.AbortPlannedMoves(); // stop rotating the pulley
            //            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
        } else if (/*tmc2130_read_gstat() &&*/ mm::motion.QueueEmpty()) {
            // we reached the end of move queue, but the FINDA didn't switch off
            // two possible causes - grinded filament or malfunctioning FINDA
            if (--maxTries) {
                Reset(maxTries); // try again
            } else {
                state = FailedFINDA;
            }
        }
    }
        return false;
    case OK:
    case FailedFINDA:
    case FailedFSensor:
    default:
        return true;
    }
}

} // namespace logic
