#include "load_filament.h"
#include "../modules/finda.h"
#include "../modules/globals.h"
#include "../modules/idler.h"
#include "../modules/leds.h"
#include "../modules/motion.h"
#include "../modules/permanent_storage.h"
#include "../modules/selector.h"
#include "../modules/user_input.h"

namespace logic {

LoadFilament loadFilament;

void LoadFilament::Reset(uint8_t param) {
    if (!CheckToolIndex(param)) {
        return;
    }

    state = ProgressCode::EngagingIdler;
    error = ErrorCode::RUNNING;
    mg::globals.SetActiveSlot(param);
    mi::idler.Engage(mg::globals.ActiveSlot());
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::blink0);
    ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
}

bool LoadFilament::StepInner() {
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
                state = ProgressCode::ERRDisengagingIdler;
                error = ErrorCode::FINDA_DIDNT_SWITCH_ON;
                mi::idler.Disengage();
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::off);
                ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::blink0); // signal loading error
            } else {
                state = ProgressCode::FeedingToBondtech;
                james.Reset(config::feedToBondtechMaxRetries);
            }
        }
        break;
    case ProgressCode::FeedingToBondtech:
        if (james.Step()) { // No, Mr. Bond, I expect you to FEED
            switch (james.State()) {
            case FeedToBondtech::Failed:

            case FeedToBondtech::OK:
                mm::motion.Disable(mm::Pulley);
                mi::idler.Disengage();
                state = ProgressCode::DisengagingIdler;
            }
        }
        break;
    case ProgressCode::DisengagingIdler:
        if (!mi::idler.Engaged()) {
            state = ProgressCode::OK;
            error = ErrorCode::OK;
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::on);
            mg::globals.SetFilamentLoaded(true);
        }
        break;
    case ProgressCode::OK:
        return true;
    case ProgressCode::ERRDisengagingIdler: // couldn't unload to FINDA
        if (!mi::idler.Engaged()) {
            state = ProgressCode::ERRWaitingForUser;
            mui::userInput.Clear(); // remove all buffered events if any just before we wait for some input
        }
        return false;
    case ProgressCode::ERRWaitingForUser: {
        // waiting for user buttons and/or a command from the printer
        mui::Event ev = mui::userInput.ConsumeEvent();
        switch (ev) {
        case mui::Event::Left: // try to manually load just a tiny bit - help the filament with the pulley
            state = ProgressCode::ERREngagingIdler;
            mi::idler.Engage(mg::globals.ActiveSlot());
            break;
        case mui::Event::Middle: // try again the whole sequence
            Reset(mg::globals.ActiveSlot());
            break;
        case mui::Event::Right: // problem resolved - the user pushed the fillament by hand?
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::red, ml::off);
            ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::on);
            //                mm::motion.PlanMove(mm::Pulley, 450, 5000); // @@TODO constants
            state = ProgressCode::AvoidingGrind;
            break;
        default: // no event, continue waiting for user input
            break;
        }
        return false;
    }
    case ProgressCode::ERREngagingIdler:
        if (mi::idler.Engaged()) {
            state = ProgressCode::ERRHelpingFilament;
            mm::motion.PlanMove(mm::Pulley, 450, 5000); //@@TODO constants
        }
        return false;
    case ProgressCode::ERRHelpingFilament:
        if (mf::finda.Pressed()) {
            // the help was enough to press the FINDA, we are ok, continue normally
            state = ProgressCode::FeedingToBondtech;
            error = ErrorCode::RUNNING;
        } else if (mm::motion.QueueEmpty()) {
            // helped a bit, but FINDA didn't trigger, return to the main error state
            state = ProgressCode::ERRDisengagingIdler;
        }
        return false;
    default: // we got into an unhandled state, better report it
        state = ProgressCode::ERRInternal;
        error = ErrorCode::INTERNAL;
        return true;
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
