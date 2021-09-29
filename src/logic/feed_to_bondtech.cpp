#include "feed_to_bondtech.h"
#include "../modules/buttons.h"
#include "../modules/fsensor.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../debug.h"

namespace logic {

void FeedToBondtech::Reset(uint8_t maxRetries) {
    dbg_logic_P(PSTR("\nFeed to Bondtech\n\n"));
    state = EngagingIdler;
    this->maxRetries = maxRetries;
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::blink0);
    mi::idler.Engage(mg::globals.ActiveSlot());
}

bool FeedToBondtech::Step() {
    switch (state) {
    case EngagingIdler:
        if (mi::idler.Engaged()) {
            dbg_logic_sprintf_P(PSTR("\nPulley start steps %u\n\n"), mm::motion.CurPosition(mm::Pulley));
            state = PushingFilament;
            mm::motion.InitAxis(mm::Pulley);
            mm::motion.PlanMove<mm::Pulley>(config::defaultBowdenLength, config::pulleyFeedrate); //@@TODO constants - there was some strange acceleration sequence in the original FW,
            // we can probably hand over some array of constants for hand-tuned acceleration + leverage some smoothing in the stepper as well
        }
        return false;
    case PushingFilament:
        dbg_logic_P(PSTR("\nFeed to Bondtech --> Pushing\n\n"));
        if (mfs::fsensor.Pressed()) {
            mm::motion.AbortPlannedMoves(); // stop pushing filament
            mi::idler.Disengage();
            state = DisengagingIdler;
        } else if (mm::motion.StallGuard(mm::Pulley)) {
            // stall guard occurred during movement - the filament got stuck
            state = Failed; // @@TODO may be even report why it failed
        } else if (mm::motion.QueueEmpty()) { // all moves have been finished and the fsensor didn't switch on
            state = Failed;
        }
        return false;
    case DisengagingIdler:
        dbg_logic_P(PSTR("\nFeed to Bondtech --> DisengagingIdler\n\n"));
        if (!mi::idler.Engaged()) {
            state = OK;
            mm::motion.Disable(mm::Pulley);
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::on);
        }
        return false;
    case OK:
        dbg_logic_P(PSTR("\nFeed to Bondtech\n\n"));
    case Failed:
        dbg_logic_P(PSTR("\nFeed to Bondtech FAILED\n\n"));
    default:
        return true;
    }
}

} // namespace logic
