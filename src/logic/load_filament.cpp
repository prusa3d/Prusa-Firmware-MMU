#include "load_filament.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../modules/selector.h"

namespace logic {

LoadFilament loadFilament;

namespace mm = modules::motion;
namespace mi = modules::idler;
namespace ms = modules::selector;
namespace mf = modules::finda;
namespace ml = modules::leds;
namespace mg = modules::globals;

void LoadFilament::Reset(uint8_t param) {
    state = ProgressCode::EngagingIdler;
    error = ErrorCode::OK;
    mg::globals.SetActiveSlot(param);
    mi::idler.Engage(mg::globals.ActiveSlot());
}

bool LoadFilament::Step() {
    switch (state) {
    case ProgressCode::EngagingIdler:
        if (mi::idler.Engaged()) {
            mm::motion.InitAxis(mm::Pulley);
            state = ProgressCode::FeedingToFinda;
            feed.Reset(true);
        }
        break;
    case ProgressCode::FeedingToFinda:
        if (feed.Step()) {
            if (feed.State() == FeedToFinda::Failed) {
                // @@TODO - try to repeat 6x - push/pull sequence - probably something to put into feed_to_finda as an option
                state = ProgressCode::ERR1DisengagingIdler;
                mi::idler.Disengage();
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::Color::red, ml::Mode::blink0); // signal loading error
            } else {
                state = ProgressCode::FeedingToBondtech;
                james.Reset(2);
            }
        }
        break;
    case ProgressCode::FeedingToBondtech:
        if (james.Step()) { // No, Mr. Bond, I expect you to FEED
            switch (james.State()) {
            case FeedToBondtech::Failed:

            case FeedToBondtech::OK:
                mm::motion.DisableAxis(mm::Pulley);
                mi::idler.Disengage();
                state = ProgressCode::DisengagingIdler;
            }
        }
    case ProgressCode::DisengagingIdler:
        if (!mi::idler.Engaged()) {
            state = ProgressCode::OK;
        }
        break;
    case ProgressCode::OK:
        mg::globals.SetFilamentLoaded(true);
        return true;
    case ProgressCode::ERR1DisengagingIdler: // couldn't unload to FINDA
        error = ErrorCode::FINDA_DIDNT_TRIGGER;
        if (!mi::idler.Engaged()) {
            state = ProgressCode::ERR1WaitingForUser;
        }
        return false;
    case ProgressCode::ERR1WaitingForUser: {
        // waiting for user buttons and/or a command from the printer
        bool help = modules::buttons::buttons.ButtonPressed(modules::buttons::Left) /*|| command_help()*/;
        bool tryAgain = modules::buttons::buttons.ButtonPressed(modules::buttons::Middle) /*|| command_tryAgain()*/;
        bool userResolved = modules::buttons::buttons.ButtonPressed(modules::buttons::Right) /*|| command_userResolved()*/;
        if (help) {
            // try to manually load just a tiny bit - help the filament with the pulley
            //@@TODO
        } else if (tryAgain) {
            // try again the whole sequence
            Reset(0); // @@TODO param
        } else if (userResolved) {
            // problem resolved - the user pulled the fillament by hand
            modules::leds::leds.SetMode(mg::globals.ActiveSlot(), modules::leds::red, modules::leds::off);
            modules::leds::leds.SetMode(mg::globals.ActiveSlot(), modules::leds::green, modules::leds::on);
            //                mm::motion.PlanMove(mm::Pulley, 450, 5000); // @@TODO constants
            state = ProgressCode::AvoidingGrind;
        }
        return false;
    }
    }
    return false;
}

//! @brief Load filament through bowden
//! @param disengageIdler
//!  * true Disengage idler after movement
//!  * false Do not disengage idler after movement
//void load_filament_withSensor(bool disengageIdler)
//{
//    FilamentLoaded::set(active_extruder);
//    motion_engage_idler();

//    tmc2130_init_axis(AX_PUL, tmc2130_mode);

//    set_pulley_dir_push();

//    int _loadSteps = 0;
//    int _endstop_hit = 0;

//    // load filament until FINDA senses end of the filament, means correctly loaded into the selector
//    // we can expect something like 570 steps to get in sensor
//    do{
//        do_pulley_step();
//        _loadSteps++;
//        delayMicroseconds(5500);
//    } while (digitalRead(A1) == 0 && _loadSteps < 1500);

//    // filament did not arrive at FINDA, let's try to correct that
//    if (digitalRead(A1) == 0){
//        for (int i = 6; i > 0; i--){
//            if (digitalRead(A1) == 0){
//                // attempt to correct
//                set_pulley_dir_pull();
//                for (int i = 200; i >= 0; i--){
//                    do_pulley_step();
//                    delayMicroseconds(1500);
//                }

//                set_pulley_dir_push();
//                _loadSteps = 0;
//                do{
//                    do_pulley_step();
//                    _loadSteps++;
//                    delayMicroseconds(4000);
//                    if (digitalRead(A1) == 1) _endstop_hit++;
//                } while (_endstop_hit<100 && _loadSteps < 500);
//            }
//        }
//    }

//    // still not at FINDA, error on loading, let's wait for user input
//    if (digitalRead(A1) == 0){
//        bool _continue = false;
//        bool _isOk = false;
//        motion_disengage_idler();
//        do{
//            if (!_isOk){
//                signal_load_failure();
//            }else{
//                signal_ok_after_load_failure();
//            }

//            switch (buttonPressed()){
//                case Btn::left:
//                    // just move filament little bit
//                    motion_engage_idler();
//                    set_pulley_dir_push();

//                    for (int i = 0; i < 200; i++)
//                    {
//                        do_pulley_step();
//                        delayMicroseconds(5500);
//                    }
//                    motion_disengage_idler();
//                    break;
//                case Btn::middle:
//                    // check if everything is ok
//                    motion_engage_idler();
//                    _isOk = checkOk();
//                    motion_disengage_idler();
//                    break;
//                case Btn::right:
//                    // continue with loading
//                    motion_engage_idler();
//                    _isOk = checkOk();
//                    motion_disengage_idler();

//                    if (_isOk) //pridat do podminky flag ze od tiskarny prislo continue
//                    {
//                        _continue = true;
//                    }
//                    break;
//                default:
//                    break;
//            }

//        } while ( !_continue );

//        motion_engage_idler();
//        set_pulley_dir_push();
//        _loadSteps = 0;
//        do
//        {
//            do_pulley_step();
//            _loadSteps++;
//            delayMicroseconds(5500);
//        } while (digitalRead(A1) == 0 && _loadSteps < 1500);
//        // ?
//    }
//    else
//    {
//        // nothing
//    }

//    motion_feed_to_bondtech();

//    tmc2130_disable_axis(AX_PUL, tmc2130_mode);
//    if (disengageIdler) motion_disengage_idler();
//    isFilamentLoaded = true;  // filament loaded

} // namespace logic
