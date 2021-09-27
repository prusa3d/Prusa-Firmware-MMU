#include "retract_from_finda.h"
#include "../modules/finda.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"

namespace logic {

void RetractFromFinda::Reset() {
    state = EngagingIdler;
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::blink0);
    mi::idler.Engage(mg::globals.ActiveSlot());
}

bool RetractFromFinda::Step() {
    switch (state) {
    case EngagingIdler:
        if (mi::idler.Engaged()) {
            state = UnloadBackToPTFE;
            mm::motion.PlanMove<mm::Pulley>(-config::cuttingEdgeToFindaMidpoint, config::pulleyFeedrate);
        }
        return false;
    case UnloadBackToPTFE:
        if (mm::motion.QueueEmpty()) { // all moves have been finished
            if (!mf::finda.Pressed()) { // FINDA switched off correctly
                state = OK;
            } else { // FINDA didn't switch off
                state = Failed;
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::blink0);
            }
        }
        return false;
    case OK:
    case Failed:
    default:
        return true;
    }
}

} // namespace logic
