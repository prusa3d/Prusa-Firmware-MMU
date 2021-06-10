#include "load_filament.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"

namespace logic {

LoadFilament loadFilament;

namespace mm = modules::motion;

void LoadFilament::Reset(uint8_t param) {
    state = ProgressCode::EngagingIdler;
    error = ErrorCode::OK;
}

bool LoadFilament::Step() {
    switch (state) {
    }
    return false;
}

} // namespace logic
