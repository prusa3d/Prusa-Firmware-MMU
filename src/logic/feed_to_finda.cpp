#include "feed_to_finda.h"
#include "../modules/finda.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/selector.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../modules/user_input.h"

namespace logic {

void FeedToFinda::Reset(bool feedPhaseLimited) {
    state = EngagingIdler;
    this->feedPhaseLimited = feedPhaseLimited;
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::blink0);
    mi::idler.Engage(mg::globals.ActiveSlot());
    // We can't get any FINDA readings if the selector is at the wrong spot - move it accordingly if necessary
    ms::selector.MoveToSlot(mg::globals.ActiveSlot());
}

bool FeedToFinda::Step() {
    switch (state) {
    case EngagingIdler:
        if (mi::idler.Engaged() && ms::selector.Slot() == mg::globals.ActiveSlot()) {
            state = PushingFilament;
            mm::motion.PlanMove<mm::Pulley>(config::feedToFinda, config::pulleyFeedrate);
            mui::userInput.Clear(); // remove all buffered events if any just before we wait for some input
        }
        return false;
    case PushingFilament: {
        if (mf::finda.Pressed() || (feedPhaseLimited && mui::userInput.AnyEvent())) { // @@TODO probably also a command from the printer
            mm::motion.AbortPlannedMoves(); // stop pushing filament
            // FINDA triggered - that means it works and detected the filament tip
            state = UnloadBackToPTFE;
            mm::motion.PlanMove<mm::Pulley>(-config::cuttingEdgeToFindaMidpoint, config::pulleyFeedrate);
        } else if (mm::motion.QueueEmpty()) { // all moves have been finished and FINDA didn't switch on
            state = Failed;
            // @@TODO - shall we disengage the idler?
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::blink0);
        }
    }
        return false;
    case UnloadBackToPTFE:
        if (mm::motion.QueueEmpty()) { // all moves have been finished
            state = DisengagingIdler;
            mi::idler.Disengage();
        }
        return false;
    case DisengagingIdler:
        if (!mi::idler.Engaged()) {
            state = OK;
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
        }
        // @@TODO FINDA must be reported as OFF again as we are pulling the filament from it - is this correct?
        return false;
    case OK:
    case Failed:
    default:
        return true;
    }
}

} // namespace logic
