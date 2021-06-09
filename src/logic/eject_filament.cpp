#include "eject_filament.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"

namespace logic {

EjectFilament ejectFilament;

void EjectFilament::Reset() {
    namespace mm = modules::motion;
    state = ProgressCode::EngagingIdler;
    error = ErrorCode::OK;
}

bool EjectFilament::Step() {
    namespace mm = modules::motion;
    switch (state) {
    }
    return false;
}

} // namespace logic
