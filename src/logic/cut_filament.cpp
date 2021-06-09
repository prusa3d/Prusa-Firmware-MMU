#include "cut_filament.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"

namespace logic {

CutFilament cutFilament;

void CutFilament::Reset() {
    namespace mm = modules::motion;
    state = ProgressCode::EngagingIdler;
    error = ErrorCode::OK;
}

bool CutFilament::Step() {
    namespace mm = modules::motion;
    switch (state) {
    }
    return false;
}

} // namespace logic
