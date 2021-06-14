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

namespace mm = modules::motion;
namespace mi = modules::idler;
namespace ms = modules::selector;
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

bool ToolChange::Step() {
    switch (state) {
    case ProgressCode::UnloadingFilament:
        if (unl.Step()) {
            // unloading sequence finished
            switch (unl.Error()) {
            case ErrorCode::OK: // finished successfully
                state = ProgressCode::LoadingFilament;
                load.Reset(plannedSlot);
                break;
            case ErrorCode::UNLOAD_ERROR2: // @@TODO what shall we do in case of this error?
            case ErrorCode::FINDA_DIDNT_TRIGGER:
                break;
            }
        }
        break;
    case ProgressCode::LoadingFilament:
        if (load.Step()) {
            // unloading sequence finished
            switch (load.Error()) {
            case ErrorCode::OK: // finished successfully
                state = ProgressCode::OK;
                break;
                //            case ErrorCode::LOAD_ERROR2: // @@TODO load errors?
                //            case ErrorCode::LOAD_FINDA_DIDNT_TRIGGER:
                break;
            }
        }
        break;
    case ProgressCode::OK:
        return true;
    }
    return false;
}

} // namespace logic
