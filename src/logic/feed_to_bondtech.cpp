#include "feed_to_bondtech.h"
#include "../modules/buttons.h"
#include "../modules/fsensor.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#ifdef DEBUG_LOGIC
    #include "../hal/usart.h"
    #include <string.h>
    #include <stdio.h>
#endif //DEBUG_LOGIC

namespace logic {

void FeedToBondtech::Reset(uint8_t maxRetries) {
    #ifdef DEBUG_LOGIC
        hu::usart1.puts("\nFeed to Bondtech\n\n");
    #endif //DEBUG_LOGIC
    state = EngagingIdler;
    this->maxRetries = maxRetries;
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::Color::green, ml::Mode::blink0);
    mi::idler.Engage(mg::globals.ActiveSlot());
}

bool FeedToBondtech::Step() {
    const uint16_t steps = mps::BowdenLength::get();

    switch (state) {
    case EngagingIdler:
        if (mi::idler.Engaged()) {
        #ifdef DEBUG_LOGIC
            uint16_t startSteps = mm::motion.CurPosition(mm::Pulley);
            char str[30];
            sprintf_P(str, PSTR("\nPulley start steps %u\n\n"), startSteps);
            hu::usart1.puts(str);
        #endif //DEBUG_LOGIC
            state = PushingFilament;
            mm::motion.PlanMove<mm::Pulley>(config::DefaultBowdenLength, config::pulleyFeedrate); //@@TODO constants - there was some strange acceleration sequence in the original FW,
            // we can probably hand over some array of constants for hand-tuned acceleration + leverage some smoothing in the stepper as well
        }
        return false;
    case PushingFilament:
        #ifdef DEBUG_LOGIC
            hu::usart1.puts("\nFeed to Bondtech --> Pushing\n\n");
        #endif //DEBUG_LOGIC
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
        #ifdef DEBUG_LOGIC
            hu::usart1.puts("\nFeed to Bondtech --> DisengagingIdler\n\n");
        #endif //DEBUG_LOGIC
        if (!mi::idler.Engaged()) {
            state = OK;
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::on);
        }
        return false;
    case OK:
    #ifdef DEBUG_LOGIC
        hu::usart1.puts("\nFeed to Bondtech\n\n");
    #endif //DEBUG_LOGIC
    case Failed:
    #ifdef DEBUG_LOGIC
        hu::usart1.puts("\nFeed to Bondtech FAILED\n\n");
    #endif //DEBUG_LOGIC
    default:
        return true;
    }
}

} // namespace logic
