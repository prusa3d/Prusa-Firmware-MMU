#include "feed_to_bondtech.h"
#include "../modules/buttons.h"
#include "../modules/fsensor.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"

namespace logic {

namespace mm = modules::motion;
namespace mfs = modules::fsensor;
namespace mi = modules::idler;
namespace ml = modules::leds;
namespace mp = modules::permanent_storage;

void FeedToBondtech::Reset(uint8_t maxRetries) {
    state = EngagingIdler;
    this->maxRetries = maxRetries;
    mi::idler.Engage(0 /*active_extruder*/); // @@TODO
}

bool FeedToBondtech::Step() {
    const uint16_t steps = mp::BowdenLength::get();

    switch (state) {
    case EngagingIdler:
        if (mi::idler.Engaged()) {
            state = PushingFilament;
            ml::leds.SetMode(0, ml::Color::green, ml::blink0); //@@TODO active slot index
            mm::motion.PlanMove(steps, 0, 0, 4500, 0, 0); //@@TODO constants - there was some strange acceleration sequence in the original FW,
            // we can probably hand over some array of constants for hand-tuned acceleration + leverage some smoothing in the stepper as well
        }
        return false;
    case PushingFilament:
        if (mfs::fsensor.Pressed()) {
            mm::motion.AbortPlannedMoves(); // stop pushing filament
            state = DisengagingIdler;
        } else if (mm::motion.StallGuard(mm::Pulley)) {
            // stall guard occurred during movement - the filament got stuck
            state = Failed; // @@TODO may be even report why it failed
        } else if (mm::motion.QueueEmpty()) { // all moves have been finished and the fsensor didn't switch on
            state = Failed;
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
