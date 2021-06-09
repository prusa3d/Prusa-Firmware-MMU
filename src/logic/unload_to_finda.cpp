#include "unload_to_finda.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"

namespace logic {

namespace mm = modules::motion;
namespace mf = modules::finda;

void UnloadToFinda::Reset() {
    // check the inital state of FINDA and plan the moves
    if (mf::finda.Pressed()) {
        state = OK; // FINDA is already off, we assume the fillament is not there, i.e. already unloaded
    } else {
        // FINDA is sensing the filament, plan moves to unload it
        int unloadSteps = /*BowdenLength::get() +*/ 1100; // @@TODO
        const int second_point = unloadSteps - 1300;
        //            mm::motion.PlanMove(mm::Pulley, -1400, 6000); // @@TODO constants
        //            mm::motion.PlanMove(mm::Pulley, -1800 + 1400, 2500); // @@TODO constants 1800-1400 = 400
        //            mm::motion.PlanMove(mm::Pulley, -second_point + 1800, 550); // @@TODO constants
        state = WaitingForFINDA;
    }
}

bool UnloadToFinda::Step() {
    switch (state) {
    case WaitingForFINDA:
        if (modules::finda::finda.Pressed()) {
            // detected end of filament
            state = OK;
        } else if (/*tmc2130_read_gstat() &&*/ mm::motion.QueueEmpty()) {
            // we reached the end of move queue, but the FINDA didn't switch off
            // two possible causes - grinded filament of malfunctioning FINDA
            if (--maxTries) {
                Reset(); // try again
            } else {
                state = Failed;
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
