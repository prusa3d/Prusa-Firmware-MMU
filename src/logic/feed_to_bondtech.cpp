#include "feed_to_bondtech.h"
#include "../modules/buttons.h"
#include "../modules/fsensor.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"

namespace logic {

void FeedToBondtech::Reset(uint8_t maxRetries) {
    state = EngagingIdler;
    this->maxRetries = maxRetries;
    mi::idler.Engage(mg::globals.ActiveSlot());
}

bool FeedToBondtech::Step() {
    const uint16_t steps = mps::BowdenLength::get();

    switch (state) {
    case EngagingIdler:
        if (mi::idler.Engaged()) {
            state = PushingFilament;
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::Color::green, ml::blink0);
            mm::motion.PlanMove(mm::Pulley, steps, 4500); //@@TODO constants - there was some strange acceleration sequence in the original FW,
            // we can probably hand over some array of constants for hand-tuned acceleration + leverage some smoothing in the stepper as well
        }
        return false;
    case PushingFilament:
        if (mfs::fsensor.Pressed()) {
            mm::motion.AbortPlannedMoves(); // stop pushing filament
            //            mi::idler.Disengage();
            state = OK;
        } else if (mm::motion.StallGuard(mm::Pulley)) {
            // stall guard occurred during movement - the filament got stuck
            state = Failed; // @@TODO may be even report why it failed
        } else if (mm::motion.QueueEmpty()) { // all moves have been finished and the fsensor didn't switch on
            state = Failed;
        }
        return false;
        //    case DisengagingIdler:
        //        if (!mi::idler.Engaged()) {
        //            state = OK;
        //            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::Color::green, ml::on);
        //        }
        //        return false;
    case OK:
    case Failed:
    default:
        return true;
    }
}

} // namespace logic
