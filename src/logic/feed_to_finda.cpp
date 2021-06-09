#include "feed_to_finda.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"

namespace logic {

namespace mm = modules::motion;
namespace mf = modules::finda;
namespace mi = modules::idler;
namespace ml = modules::leds;
namespace mb = modules::buttons;

void FeedToFinda::Reset(bool feedPhaseLimited) {
    state = EngagingIdler;
    this->feedPhaseLimited = feedPhaseLimited;
    mi::idler.Engage(0 /*active_extruder*/); // @@TODO
}

bool FeedToFinda::Step() {
    switch (state) {
    case EngagingIdler:
        if (mi::idler.Engaged()) {
            state = PushingFilament;
            ml::leds.SetMode(0, ml::Color::green, ml::blink0); //@@TODO active slot index
            mm::motion.PlanMove(feedPhaseLimited ? 1500 : 65535, 0, 0, 4000, 0, 0); //@@TODO constants
        }
        return false;
    case PushingFilament:
        if (mf::finda.Pressed() || (feedPhaseLimited && mb::buttons.AnyButtonPressed())) { // @@TODO probably also a command from the printer
            mm::motion.AbortPlannedMoves(); // stop pushing filament
            // FINDA triggered - that means it works and detected the filament tip
            state = UnloadBackToPTFE;
            mm::motion.PlanMove(-600, 0, 0, 4000, 0, 0); //@@TODO constants
        } else if (mm::motion.QueueEmpty()) { // all moves have been finished and FINDA didn't switch on
            state = Failed;
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
            ml::leds.SetMode(0, ml::Color::green, ml::on); //@@TODO active slot index
        }
        return false;
    case OK:
    case Failed:
    default:
        return true;
    }
}

} // namespace logic
