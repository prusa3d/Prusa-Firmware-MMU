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
#include "../modules/timebase.h"

namespace {
  // Local parameters; if you prefer, move them to config::
  constexpr uint16_t kFsensorWaitTimeout_ms = 4000; // how long we wait for fsensor==OFF before micro-pull
  constexpr uint8_t  kMaxMicroPullTries     = 2;    // number of 1mm micro-pull attempts
  constexpr unit::U_mm kMicroPull_mm          = 1.0_mm;  // distance of each micro-pull
}

namespace logic {

void UnloadToFinda::Reset(uint8_t maxTries) {
  this->maxTries = maxTries;
  // check the inital state of FINDA and plan the moves
  if (!mf::finda.Pressed()) {
    state = OK; // FINDA is already off, we assume the filament is not there, i.e. already unloaded
  } else {
    // FINDA is sensing the filament, plan moves to unload it
    state = EngagingIdler;
    mi::idler.PartiallyDisengage(mg::globals.ActiveSlot()); // prepare before the active slot - saves ~1s
    started_ms = 0;               // start timeout later, when we actually wait for fsensor==OFF
    microPullTries = 0;
    microPullMovePlanned = false;
    ml::leds.ActiveSlotProcessing();
  }
}

bool UnloadToFinda::Step() {
  switch (state) {
  // start by engaging the idler into intermediate position
  // Then, wait for !fsensor.Pressed: to speed-up the pull process - unload operation will be started during the purging moves
  // and as soon as the fsensor turns off, the MMU engages the idler fully and starts pulling.
  // It will not wait for the extruder to finish the relieve move.
  case EngagingIdler:
    if (!mi::idler.PartiallyDisengaged()) { // just waiting for Idler to get into the target intermediate position
      return false;
    }
    if (mfs::fsensor.Pressed()) { // still pressed, printer didn't free the filament yet
      // Start the timeout NOW (not in Reset), only when we actually wait for fsensor==OFF
      if (started_ms == 0) started_ms = mt::timebase.Millis();
      // If timeout elapsed, trigger micro-pull strategy with idler fully engaged
      if (mt::timebase.Elapsed(started_ms, kFsensorWaitTimeout_ms)) {
        mpu::pulley.InitAxis();
        mi::idler.Engage(mg::globals.ActiveSlot());
        microPullMovePlanned = false; // plan in MicroPullTry
        state = MicroPullTry;
      }
      return false;
    } else {
      // fsensor is OFF and Idler is partially engaged, engage the Idler fully and pull
      if (mg::globals.FilamentLoaded() >= mg::FilamentLoadState::InSelector) {
        state = UnloadingToFinda;
        mpu::pulley.InitAxis();
        mi::idler.Engage(mg::globals.ActiveSlot());
        // slow move for the first few millimeters - help the printer relieve the filament while engaging the Idler fully
        mpu::pulley.PlanMove(-config::fsensorToNozzleAvoidGrindUnload,
                             mg::globals.PulleySlowFeedrate_mm_s(),
                             mg::globals.PulleySlowFeedrate_mm_s());
      } else {
        state = FailedFINDA;
      }
    }
    return false;

  case MicroPullTry:
    // Ensure the Idler is fully engaged before moving
    if (!mi::idler.Engaged()) {
      return false;
    }
    // Plan the 1mm micro-pull only once per attempt
    if (!microPullMovePlanned) {
      mpu::pulley.PlanMove(-kMicroPull_mm,
                           mg::globals.PulleySlowFeedrate_mm_s(),
                           mg::globals.PulleySlowFeedrate_mm_s());
      microPullMovePlanned = true;
      return false;
    }
    // Wait for the micro move to finish
    if (!mm::motion.QueueEmpty()) {
      return false;
    }
    // After the micro-pull, check FSensor
    if (!mfs::fsensor.Pressed()) {
      // FSensor finally OFF -> proceed with normal unload
      if (mg::globals.FilamentLoaded() >= mg::FilamentLoadState::InSelector) {
        state = UnloadingToFinda;
        // The UnloadingToFinda state will plan the long unload move
      } else {
        state = FailedFINDA;
      }
      microPullMovePlanned = false;
      return false;
    }
    // Still ON -> try again up to kMaxMicroPullTries
    if (++microPullTries < kMaxMicroPullTries) {
      microPullMovePlanned = false; // schedule another micro-pull
      return false;
    } else {
      // attempts exhausted -> fail safely
      state = FailedFSensor;
      microPullMovePlanned = false;
      return true;
    }

  case UnloadingToFinda:
    if (mi::idler.Engaged()) {
      state = WaitingForFINDA;
      mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::FilamentLoadState::InSelector);
      unloadStart_mm = mpu::pulley.CurrentPosition_mm();
      // We can always plan the unload move for the maximum allowed bowden length,
      // it should be even more reliable than doing just the specified bowden length:
      // - if the filament is slipping for some reason, planning a longer move will not stop in the middle of the bowden tube
      // - a faster unload (shorter than the specified bowden length) will be interrupted by FINDA turning off
      // - if FINDA is misaligned or faulty, the only issue will be, that the filament will be thrown behind the pulley
      //   which could have happened with the previous implementation as well, because default bowden length was set to 42cm
      mpu::pulley.PlanMove(-config::maximumBowdenLength - config::feedToFinda - config::filamentMinLoadedToMMU,
                           mg::globals.PulleyUnloadFeedrate_mm_s());
    }
    return false;

  case WaitingForFINDA: {
    int32_t currentPulley_mm = mpu::pulley.CurrentPosition_mm();
    if ((abs(unloadStart_mm - currentPulley_mm) > mm::truncatedUnit(mg::globals.FSensorUnloadCheck_mm())) && mfs::fsensor.Pressed()) {
      // fsensor didn't trigger within the first fsensorUnloadCheckDistance mm -> stop pulling, something failed, report an error
      // This scenario should not be tried again - repeating it may cause more damage to filament + potentially more collateral damage
      state = FailedFSensor;
      mm::motion.AbortPlannedMoves(); // stop rotating the pulley
      ml::leds.ActiveSlotDoneEmpty();
    } else if (!mf::finda.Pressed()) {
      // detected end of filament
      state = OK;
      mm::motion.AbortPlannedMoves(); // stop rotating the pulley
      ml::leds.ActiveSlotDoneEmpty();
    } else if (/*tmc2130_read_gstat() &&*/ mm::motion.QueueEmpty()) {
      // we reached the end of move queue, but the FINDA didn't switch off
      // two possible causes - grinded filament or malfunctioning FINDA
      if (--maxTries) {
        // Ideally, the Idler shall rehome and then try again.
        // That would auto-resolve errors caused by slipped or misaligned Idler
        mi::idler.InvalidateHoming();
        Reset(maxTries);
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
