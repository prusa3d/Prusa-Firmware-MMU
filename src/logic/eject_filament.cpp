#include "eject_filament.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../modules/selector.h"

namespace logic {

EjectFilament ejectFilament;

namespace mm = modules::motion;
namespace mi = modules::idler;
namespace ms = modules::selector;

void EjectFilament::Reset(uint8_t param) {
    error = ErrorCode::OK;
    slot = param;

    bool isFilamentLoaded = true; //@@TODO

    if (isFilamentLoaded) {
        state = ProgressCode::UnloadingFilament;
        unl.Reset(param); //@@TODO probably act on active extruder only
    } else {
        MoveSelectorAside();
    }
}

void EjectFilament::MoveSelectorAside() {
    state = ProgressCode::ParkingSelector;
    const uint8_t selectorParkedPos = (slot <= 2) ? 4 : 0;
    mi::idler.Engage(slot);
    ms::selector.MoveToSlot(selectorParkedPos);
}

bool EjectFilament::Step() {
    constexpr const uint16_t eject_steps = 500; //@@TODO
    switch (state) {
    case ProgressCode::UnloadingFilament:
        if (unl.Step()) {
            // unloading sequence finished
            switch (unl.Error()) {
            case ErrorCode::OK: // finished successfully
            case ErrorCode::UNLOAD_ERROR2: // @@TODO what shall we do in case of this error?
            case ErrorCode::FINDA_DIDNT_TRIGGER:
                break;
            }
        }
        break;
    case ProgressCode::ParkingSelector:
        if (mm::motion.QueueEmpty()) { // selector parked aside
            state = ProgressCode::EjectingFilament;
            mm::motion.InitAxis(mm::Pulley);
            mm::motion.PlanMove(eject_steps, 0, 0, 1500, 0, 0);
        }
        break;
    case ProgressCode::EjectingFilament:
        if (mm::motion.QueueEmpty()) { // filament ejected
            state = ProgressCode::DisengagingIdler;
            mi::idler.Disengage();
        }
        break;
    case ProgressCode::DisengagingIdler:
        if (mm::motion.QueueEmpty()) { // idler disengaged
            mm::motion.DisableAxis(mm::Pulley);
            state = ProgressCode::OK;
        }
        break;
    case ProgressCode::OK:
        return true;
    }
    return false;
}

} // namespace logic
