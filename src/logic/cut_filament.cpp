#include "cut_filament.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../modules/selector.h"

namespace logic {

CutFilament cutFilament;

namespace mm = modules::motion;
namespace mi = modules::idler;
namespace ms = modules::selector;
namespace mg = modules::globals;

void CutFilament::Reset(uint8_t param) {
    error = ErrorCode::OK;

    if (mg::globals.FilamentLoaded()) {
        state = ProgressCode::UnloadingFilament;
        unl.Reset(mg::globals.ActiveSlot());
    } else {
        SelectFilamentSlot();
    }
}

void CutFilament::SelectFilamentSlot() {
    state = ProgressCode::SelectingFilamentSlot;
    uint8_t newFilamentSlot = mg::globals.ActiveSlot() + 1; // move 1 slot aside
    mi::idler.Engage(newFilamentSlot); //@@TODO does this make sense?
    ms::selector.MoveToSlot(newFilamentSlot);
}

bool CutFilament::Step() {
    const int cut_steps_pre = 700;
    const int cut_steps_post = 150;

    switch (state) {
    case ProgressCode::UnloadingFilament:
        if (unl.Step()) {
            // unloading sequence finished
            switch (unl.Error()) {
            case ErrorCode::OK: // finished successfully
            case ErrorCode::UNLOAD_ERROR2: // @@TODO what shall we do in case of this error?
            case ErrorCode::FINDA_DIDNT_TRIGGER:
                break;
            default:
                state = ProgressCode::ERRInternal;
                break;
            }
        }
        break;
    case ProgressCode::SelectingFilamentSlot:
        if (mm::motion.QueueEmpty()) { // idler and selector finished their moves
            feed.Reset(true);
            state = ProgressCode::FeedingToFinda;
        }
        break;
    case ProgressCode::FeedingToFinda: // @@TODO this state will be reused for repeated cutting of filament ... probably there will be multiple attempts, not sure
        if (feed.Step()) {
            if (feed.State() == FeedToFinda::Failed) {
                // @@TODO
            } else {
                // move selector aside - prepare the blade into active position
                state = ProgressCode::PreparingBlade;
                ms::selector.MoveToSlot(mg::globals.ActiveSlot());
            }
        }
        break;
    case ProgressCode::PreparingBlade:
        if (mm::motion.QueueEmpty()) {
            state = ProgressCode::EngagingIdler;
            mi::idler.Engage(mg::globals.ActiveSlot());
        }
        break;
    case ProgressCode::EngagingIdler:
        if (mi::idler.Engaged()) {
            state = ProgressCode::PushingFilament;
            mm::motion.PlanMove(cut_steps_pre, 0, 0, 1500, 0, 0); //@@TODO
        }
        break;
    case ProgressCode::PushingFilament:
        if (mm::motion.QueueEmpty()) {
            state = ProgressCode::PerformingCut;
            ms::selector.MoveToSlot(0);
        }
        break;
    case ProgressCode::PerformingCut:
        if (mm::motion.QueueEmpty()) { // this may not be necessary if we want the selector and pulley move at once
            state = ProgressCode::ReturningSelector;
            ms::selector.MoveToSlot(mg::globals.ActiveSlot()); // return selector back
        }
        break;
    case ProgressCode::ReturningSelector:
        if (mm::motion.QueueEmpty()) { // selector returned to position, feed the filament back to FINDA
            state = ProgressCode::FeedingToFinda;
            feed.Reset(true);
        }
        break;
    default: // we got into an unhandled state, better report it
        state = ProgressCode::ERRInternal;
        error = ErrorCode::INTERNAL;
        return true;
    }
    return false;
}

} // namespace logic
