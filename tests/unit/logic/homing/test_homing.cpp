#include "catch2/catch.hpp"

#include "../../../../src/modules/buttons.h"
#include "../../../../src/modules/finda.h"
#include "../../../../src/modules/fsensor.h"
#include "../../../../src/modules/globals.h"
#include "../../../../src/modules/idler.h"
#include "../../../../src/modules/leds.h"
#include "../../../../src/modules/motion.h"
#include "../../../../src/modules/permanent_storage.h"
#include "../../../../src/modules/selector.h"

#include "../../../../src/logic/home.h"
#include "../../../../src/logic/move_selector.h"
#include "../../../../src/logic/load_filament.h"
#include "../../../../src/logic/unload_filament.h"

#include "../../modules/stubs/stub_adc.h"

#include "../stubs/homing.h"
#include "../stubs/main_loop_stub.h"
#include "../stubs/stub_motion.h"

using Catch::Matchers::Equals;

#include "../helpers/helpers.ipp"

bool SuccessfulHome(uint8_t slot) {
    // prepare startup conditions
    ForceReinitAllAutomata();

    // change the startup to what we need here
    EnsureActiveSlotIndex(slot, mg::FilamentLoadState::AtPulley);

    // set FINDA OFF + debounce
    SetFINDAStateAndDebounce(false);

    logic::Home h;
    REQUIRE(VerifyState(h, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), slot, false, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));

    h.Reset(0);
    REQUIRE(VerifyState(h, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), slot, false, false, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::Homing));
    REQUIRE_FALSE(mi::idler.HomingValid());
    REQUIRE_FALSE(ms::selector.HomingValid());

    SimulateIdlerAndSelectorHoming(h);

    REQUIRE(WhileTopState(h, ProgressCode::Homing, 5000));

    REQUIRE(VerifyState(h, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), slot, false, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));
    REQUIRE(mi::idler.HomingValid());
    REQUIRE(ms::selector.HomingValid());

    return true;
}

TEST_CASE("homing::successful_run", "[homing]") {
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        REQUIRE(SuccessfulHome(slot));
    }
}

bool SelectorFailedRetry() {
    // prepare startup conditions
    ForceReinitAllAutomata();

    // change the startup to what we need here
    EnsureActiveSlotIndex(0, mg::FilamentLoadState::AtPulley);

    // set FINDA OFF + debounce
    SetFINDAStateAndDebounce(false);

    logic::Home h;
    REQUIRE(VerifyState(h, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), 0, false, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));

    h.Reset(0);
    REQUIRE(VerifyState(h, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), 0, false, false, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::Homing));

    REQUIRE(SimulateFailedHomeFirstTime(h));

    for (uint8_t i = 0; i < 5; ++i) {
        REQUIRE(SimulateFailedHomeSelectorRepeated(h));
    }

    SimulateSelectorHoming(h);

    REQUIRE(WhileTopState(h, ProgressCode::Homing, 5000));

    REQUIRE(VerifyState(h, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), 0, false, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));
    REQUIRE(mi::idler.HomingValid());
    REQUIRE(ms::selector.HomingValid());

    return true;
}

TEST_CASE("homing::selector_failed_retry", "[homing]") {
    REQUIRE(SelectorFailedRetry());
}

bool SimulateFailedMoveFirstTime(logic::CommandBase &cb) {
    {
        // do 5 steps until we trigger the simulated stallguard
        for (uint8_t i = 0; i < 5; ++i) {
            main_loop();
            cb.Step();
        }

        mm::TriggerStallGuard(mm::Selector);
        main_loop();
        cb.Step();
        mm::motion.StallGuardReset(mm::Selector);
    }

    while (ms::selector.State() != mm::MovableBase::MoveFailed) {
        main_loop();
        cb.Step();
    }

    REQUIRE_FALSE(!WhileTopState(cb, ProgressCode::MovingSelector, 5));

    REQUIRE(cb.Error() == ErrorCode::MOVE_SELECTOR_FAILED);
    REQUIRE(cb.State() == ProgressCode::ERRWaitingForUser);
    //    REQUIRE_FALSE(mm::motion.Enabled(mm::Selector));

    // do a few steps before pushing the button
    WhileTopState(cb, ProgressCode::ERRWaitingForUser, 5);

    //    REQUIRE_FALSE(mm::motion.Enabled(mm::Selector));

    PressButtonAndDebounce(cb, mb::Middle);

    // it shall start homing again
    REQUIRE(cb.Error() == ErrorCode::RUNNING);
    REQUIRE(cb.State() == ProgressCode::Homing);
    REQUIRE_FALSE(ms::selector.HomingValid());
    REQUIRE(mm::motion.Enabled(mm::Selector));

    ClearButtons(cb);

    return true;
}

bool SelectorMoveFailedRetry(uint8_t fromSlot, uint8_t toSlot) {
    // prepare startup conditions
    ForceReinitAllAutomata();

    // change the startup to what we need here
    EnsureActiveSlotIndex(fromSlot, mg::FilamentLoadState::AtPulley);

    // set FINDA OFF + debounce
    SetFINDAStateAndDebounce(false);

    logic::MoveSelector ms;
    REQUIRE(VerifyState(ms, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), fromSlot, false, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));

    ms.Reset(toSlot);
    REQUIRE(VerifyState(ms, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), fromSlot, false, false, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::MovingSelector));

    REQUIRE(SimulateFailedMoveFirstTime(ms));

    //    for (uint8_t i = 0; i < 5; ++i) {
    //        REQUIRE(SimulateFailedHomeSelectorRepeated(h));
    //    }

    // both selector and idler shall home
    SimulateIdlerAndSelectorHoming(ms);

    REQUIRE(WhileTopState(ms, ProgressCode::Homing, 5000));

    REQUIRE(VerifyState(ms, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), toSlot, false, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));
    REQUIRE(mi::idler.HomingValid());
    REQUIRE(ms::selector.HomingValid());

    return true;
}

TEST_CASE("moving::selector_stallguard", "[moving]") {
    for (uint8_t from = 0; from < config::toolCount; ++from) {
        for (uint8_t to = 0; to < config::toolCount; ++to) {
            if (from != to) {
                REQUIRE(SelectorMoveFailedRetry(from, to));
            }
        }
    }
}
