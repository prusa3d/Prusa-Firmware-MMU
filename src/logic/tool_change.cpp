#include "tool_change.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"

namespace logic {

ToolChange toolChange;

void ToolChange::Reset() {
    namespace mm = modules::motion;
    state = ProgressCode::EngagingIdler;
    error = ErrorCode::OK;
}

bool ToolChange::Step() {
    namespace mm = modules::motion;
    switch (state) {
    }
    return false;
}

} // namespace logic
