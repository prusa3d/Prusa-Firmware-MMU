#include "tool_change.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../modules/selector.h"

namespace logic {

ToolChange toolChange;

namespace mg = modules::globals;

void ToolChange::Reset(uint8_t param) {
    if (param == mg::globals.ActiveSlot())
        return;

    plannedSlot = param;

    if (mg::globals.FilamentLoaded()) {
        state = ProgressCode::UnloadingFilament;
        unl.Reset(mg::globals.ActiveSlot());
    } else {
        state = ProgressCode::LoadingFilament;
        load.Reset(plannedSlot);
    }
}

bool ToolChange::StepInner() {
    switch (state) {
    case ProgressCode::UnloadingFilament:
        if (unl.StepInner()) {
            // unloading sequence finished - basically, no errors can occurr here
            // as UnloadFilament should handle all the possible error states on its own
            // There is no way the UnloadFilament to finish in an error state
            state = ProgressCode::LoadingFilament;
            load.Reset(plannedSlot);
        }
        break;
    case ProgressCode::LoadingFilament:
        if (load.StepInner()) {
            // loading sequence finished - basically, no errors can occurr here
            // as LoadFilament should handle all the possible error states on its own
            // There is no way the LoadFilament to finish in an error state
            state = ProgressCode::OK;
        }
        break;
    case ProgressCode::OK:
        return true;
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
    case ProgressCode::LoadingFilament:
        return load.State(); // report sub-automaton states properly
    default:
        return state;
    }
}

ErrorCode ToolChange::Error() const {
    switch (state) {
    case ProgressCode::UnloadingFilament:
        return unl.Error(); // report sub-automaton errors properly
    case ProgressCode::LoadingFilament:
        return load.Error(); // report sub-automaton errors properly
    default:
        return error;
    }
}

} // namespace logic
