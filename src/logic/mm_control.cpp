#include "mm_control.h"
#include "../modules/motion.h"
#include "../modules/leds.h"
#include "../modules/buttons.h"

namespace logic {

// treba tyto zakladni male automaty poslouzi k dalsim ucelum
// - engage/disengage idler
// - rotate pulley to some direction as long as the FINDA is on/off
// - rotate some axis to some fixed direction
// - unload to finda
//
// ten automat musi dat vedet, ze jeste bezi nebo ze uz skoncil
// hlavni unload filament se podle toho prepne do nejakeho jineho stavu
// budou tedy 2 kroky
// - zadani maleho automatu
// - if( skoncil )
//     nastav jiny automat, updatuj progress operace atd.
//   else
//     cekej (pripadne updatuj progress taky)

void Logic::UnloadFilament() {
    // unloads filament from extruder - filament is above Bondtech gears
    modules::motion::motion.InitAxis(modules::motion::Pulley);

    // state 1 engage idler
    modules::motion::motion.Idler(modules::motion::Motion::Engage); // if idler is in parked position un-park him get in contact with filament

    // state 2 rotate pulley as long as the FINDA is on
    if (modules::finda::active()) {
        motion_unload_to_finda();
    } else {
        if (checkOk()) {
            modules::motion::motion.Idler(modules::motion::Motion::Disengage);
            return;
        }
    }

    // state 3 move a little bit so it is not a grinded hole in filament
    modules::motion::motion.PlanMove(modules::motion::Pulley, -100, 10); // @@TODO constants

    // state 4 WTF??? FINDA is still sensing filament, let's try to unload it once again
    if (modules::finda::active()) {
        // try it 6 times
        for (int i = 6; i > 0; i--) {
            if (modules::finda::active()) {
                // set_pulley_dir_push();
                modules::motion::motion.PlanMove(modules::motion::Pulley, 150, 3000); // @@TODO constants
                // set_pulley_dir_pull();

                // cancel move if FINDA switched off

                //                int _steps = 4000;
                //                uint8_t _endstop_hit = 0;
                //                do {
                //                    do_pulley_step();
                //                    _steps--;
                //                    delayMicroseconds(3000);
                //                    if (! modules::finda::active()) _endstop_hit++;
                //                } while (_endstop_hit < 100 && _steps > 0);
            }
            //            delay(100);
        }
    }

    // state 5 error, wait for user input
    if (modules::finda::active()) {
        bool _continue = false;
        bool _isOk = false;

        modules::motion::motion.Idler(modules::motion::Motion::Disengage);

        modules::leds::leds.SetMode(active_extruder, modules::leds::red, modules::leds::blink0);

        do {
            if (modules::buttons::buttons.ButtonPressed(modules::buttons::Left)) {
                // just move filament little bit
                modules::motion::motion.Idler(modules::motion::Motion::Engage);
                // set_pulley_dir_pull();

                modules::motion::motion.PlanMove(modules::motion::Pulley, 200, 10); // @@TODO constants
                //                for (int i = 0; i < 200; i++) {
                //                    do_pulley_step();
                //                    delayMicroseconds(5500);
                //                }

                // wait for move to finish

                modules::motion::motion.Idler(modules::motion::Motion::Disengage);
            } else if (modules::buttons::buttons.ButtonPressed(modules::buttons::Middle)) {
                // check if everything is ok
                modules::motion::motion.Idler(modules::motion::Motion::Engage);
                _isOk = checkOk();
                modules::motion::motion.Idler(modules::motion::Motion::Disengage);
            } else if (modules::buttons::buttons.ButtonPressed(modules::buttons::Right)) {
                // continue with unloading
                modules::motion::motion.Idler(modules::motion::Motion::Engage);
                _isOk = checkOk();
                modules::motion::motion.Idler(modules::motion::Motion::Disengage);
            }
            if (_isOk) {
                _continue = true;
            }
        } while (!_continue);

        //shr16_set_led(1 << 2 * (4 - active_extruder));
        modules::leds::leds.SetMode(active_extruder, modules::leds::red, modules::leds::off);
        modules::leds::leds.SetMode(active_extruder, modules::leds::green, modules::leds::on);
        modules::motion::motion.Idler(modules::motion::Motion::Engage);
    } else {
        // correct unloading
        // unload to PTFE tube
        // set_pulley_dir_pull();
        modules::motion::motion.PlanMove(modules::motion::Pulley, 450, 10); // @@TODO constants
        // wait for move to finish

        //        for (int i = 450; i > 0; i--)   // 570
        //        {
        //            do_pulley_step();
        //            delayMicroseconds(5000);
        //        }
    }

    modules::motion::motion.Idler(modules::motion::Motion::Disengage);

    modules::motion::motion.DisableAxis(modules::motion::Pulley);

    isFilamentLoaded = false; // filament unloaded
}

} // namespace logic
