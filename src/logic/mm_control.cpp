#include "mm_control.h"
#include "../modules/motion.h"
#include "../modules/leds.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/permanent_storage.h"

namespace logic {

// "small" state machines will serve as building blocks for high-level commands/operations
// - engage/disengage idler
// - rotate pulley to some direction as long as the FINDA is on/off
// - rotate some axis to some fixed direction
// - load/unload to finda
//

// motion planning
// - we need some kind of planner buffer, especially because of accelerations
//   because we may need to match the ramps between moves seamlessly - just like on a printer

/// A "small" automaton example - Try to unload filament to FINDA and if it fails try to recover several times.
/// \dot
/// digraph example {
///    node [shape=record, fontname=Helvetica, fontsize=10];
///    b [ label="class B" URL="\ref B"];
///    c [ label="class C" URL="\ref C"];
///    b -> c [ arrowhead="open", style="dashed" ];
///}
///\enddot
struct UnloadToFinda {
    enum {
        WaitingForFINDA,
        OK,
        Failed
    };
    uint8_t state;
    uint8_t maxTries;
    inline UnloadToFinda(uint8_t maxTries)
        : maxTries(maxTries) { Reset(); }

    /// Restart the automaton
    inline void Reset() {
        namespace mm = modules::motion;
        namespace mf = modules::finda;
        // check the inital state of FINDA and plan the moves
        if (mf::finda.Status() == mf::Off) {
            state = OK; // FINDA is already off, we assume the fillament is not there, i.e. already unloaded
        } else {
            // FINDA is sensing the filament, plan moves to unload it
            int unloadSteps = /*BowdenLength::get() +*/ 1100; // @@TODO
            const int second_point = unloadSteps - 1300;
            mm::motion.PlanMove(mm::Pulley, -1400, 6000); // @@TODO constants
            mm::motion.PlanMove(mm::Pulley, -1800 + 1400, 2500); // @@TODO constants 1800-1400 = 400
            mm::motion.PlanMove(mm::Pulley, -second_point + 1800, 550); // @@TODO constants
            state = WaitingForFINDA;
        }
    }

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() {
        namespace mm = modules::motion;
        namespace mf = modules::finda;
        switch (state) {
        case WaitingForFINDA:
            if (modules::finda::finda.Status() == modules::finda::Off) {
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
};

/// A high-level command state machine
/// Handles the complex logic of unloading filament
class UnloadFilament : public TaskBase {
    enum State {
        EngagingIdler,
        UnloadingToFinda,
        DisengagingIdler,
        AvoidingGrind,
        Finishing,
        OK,
        ERR1DisengagingIdler,
        ERR1WaitingForUser
    };

    UnloadToFinda unl;

    inline UnloadFilament()
        : TaskBase()
        , unl(3) { Reset(); }

    /// Restart the automaton
    void Reset() override {
        namespace mm = modules::motion;
        // unloads filament from extruder - filament is above Bondtech gears
        mm::motion.InitAxis(mm::Pulley);
        state = EngagingIdler;
        mm::motion.Idler(mm::Engage);
    }

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() override {
        namespace mm = modules::motion;
        switch (state) {
        case EngagingIdler: // state 1 engage idler
            if (mm::motion.IdlerEngaged()) { // if idler is in parked position un-park him get in contact with filament
                state = UnloadingToFinda;
                unl.Reset();
            }
            return false;
        case UnloadingToFinda: // state 2 rotate pulley as long as the FINDA is on
            if (unl.Step()) {
                if (unl.state == UnloadToFinda2::Failed) {
                    // couldn't unload to FINDA, report error and wait for user to resolve it
                    state = ERR1DisengagingIdler;
                    modules::leds::leds.SetMode(active_extruder, modules::leds::red, modules::leds::blink0);
                } else {
                    state = DisengagingIdler;
                }
                // in all cases disengage the idler
                mm::motion.Idler(mm::Disengage);
            }
            return false;
        case DisengagingIdler:
            if (mm::motion.IdlerDisengaged()) {
                state = AvoidingGrind;
                mm::motion.PlanMove(mm::Pulley, -100, 10); // @@TODO constants
            }
            return false;
        case AvoidingGrind: // state 3 move a little bit so it is not a grinded hole in filament
            if (mm::motion.QueueEmpty()) {
                state = Finishing;
                mm::motion.Idler(mm::Disengage);
                return true;
            }
            return false;
        case Finishing:
            if (mm::motion.QueueEmpty()) {
                state = OK;
                mm::motion.DisableAxis(mm::Pulley);
            }
            return false;
        case ERR1DisengagingIdler: // couldn't unload to FINDA
            if (mm::motion.IdlerDisengaged()) {
                state = ERR1WaitingForUser;
            }
            return false;
        case ERR1WaitingForUser:
            // waiting for user buttons and/or a command from the printer
            bool help = modules::buttons::buttons.ButtonPressed(modules::buttons::Left) /*|| command_help()*/;
            bool tryAgain = modules::buttons::buttons.ButtonPressed(modules::buttons::Middle) /*|| command_tryAgain()*/;
            bool userResolved = modules::buttons::buttons.ButtonPressed(modules::buttons::Right) /*|| command_userResolved()*/;
            if (help) {
                // try to manually unload just a tiny bit - help the filament with the pulley
                //@@TODO
            } else if (tryAgain) {
                // try again the whole sequence
                Reset();
            } else if (userResolved) {
                // problem resolved - the user pulled the fillament by hand
                modules::leds::leds.SetMode(active_extruder, modules::leds::red, modules::leds::off);
                modules::leds::leds.SetMode(active_extruder, modules::leds::green, modules::leds::on);
                mm::motion.PlanMove(mm::Pulley, 450, 5000); // @@TODO constants
                state = AvoidingGrind;
            }
            return false;
        case OK:
            isFilamentLoaded = false; // filament unloaded
            return true; // successfully finished
        }
    }

    /// @returns progress of operation
    virtual uint8_t Progress() const override {
        return state; // for simplicity return state, will be more elaborate later in order to report the exact state of the MMU into the printer
    }
};

} // namespace logic
