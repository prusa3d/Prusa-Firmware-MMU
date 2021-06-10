#include "cut_filament.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
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

void CutFilament::Reset(uint8_t param) {
    error = ErrorCode::OK;

    bool isFilamentLoaded = true; //@@TODO

    if (isFilamentLoaded) {
        state = ProgressCode::UnloadingFilament;
        unl.Reset(param); //@@TODO probably only act on active_extruder
    } else {
        SelectFilamentSlot();
    }
}

void CutFilament::SelectFilamentSlot() {
    state = ProgressCode::SelectingFilamentSlot;
    uint8_t newFilamentSlot = 0; //@@TODO
    mi::idler.Engage(newFilamentSlot);
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
            case ErrorCode::UNLOAD_FINDA_DIDNT_TRIGGER:
                break;
            }
        }
        break;
    case ProgressCode::SelectingFilamentSlot:
        if (mm::motion.QueueEmpty()) { // idler and selector finished their moves
            feed.Reset(true);
            state = ProgressCode::FeedingToFINDA;
        }
        break;
    case ProgressCode::FeedingToFINDA: // @@TODO this state will be reused for repeated cutting of filament ... probably there will be multiple attempts, not sure
        if (feed.Step()) {
            if (feed.State() == FeedToFinda::Failed) {
                // @@TODO
            } else {
                // move selector aside - prepare the blade into active position
                state = ProgressCode::PreparingBlade;
                uint8_t newFilamentSlot = 1; //@@TODO
                ms::selector.MoveToSlot(newFilamentSlot + 1);
            }
        }
        break;
    case ProgressCode::PreparingBlade:
        if (mm::motion.QueueEmpty()) {
            state = ProgressCode::EngagingIdler;
            uint8_t newFilamentSlot = 0; //@@TODO
            mi::idler.Engage(newFilamentSlot);
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
            uint8_t newFilamentSlot = 0; //@@TODO
            ms::selector.MoveToSlot(newFilamentSlot); // return selector back
        }
        break;
    case ProgressCode::ReturningSelector:
        if (mm::motion.QueueEmpty()) { // selector returned to position, feed the filament back to FINDA
            state = ProgressCode::FeedingToFINDA;
            feed.Reset(true);
        }
        break;
    }
    return false;
}

} // namespace logic
