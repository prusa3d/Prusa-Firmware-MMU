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
#include "../../../../src/logic/load_filament.h"
#include "../../../../src/logic/unload_filament.h"

#include "../../modules/stubs/stub_adc.h"

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

    SimulateIdlerAndSelectorHoming();

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

template <typename T>
bool SimulateFailedHomePostfix(T &h) {
    REQUIRE(WhileTopState(h, ProgressCode::Homing, 5));
    REQUIRE(mi::idler.HomingValid());

    REQUIRE(h.Error() == ErrorCode::HOMING_SELECTOR_FAILED);
    REQUIRE(h.State() == ProgressCode::ERRWaitingForUser);
    REQUIRE_FALSE(mm::motion.Enabled(mm::Selector));

    // do a few steps before pushing the button
    WhileTopState(h, ProgressCode::ERRWaitingForUser, 5);

    REQUIRE_FALSE(mm::motion.Enabled(mm::Selector));

    PressButtonAndDebounce(h, mb::Middle);

    // it shall start homing again
    REQUIRE(h.Error() == ErrorCode::RUNNING);
    REQUIRE(h.State() == ProgressCode::Homing);
    REQUIRE_FALSE(ms::selector.HomingValid());
    REQUIRE(mm::motion.Enabled(mm::Selector));

    ClearButtons(h);

    return true;
}

template <typename T>
bool SimulateFailedHomeFirstTime(T &h) {
    REQUIRE_FALSE(mi::idler.HomingValid());
    REQUIRE_FALSE(ms::selector.HomingValid());

    {
        // do 5 steps until we trigger the simulated stallguard
        for (uint8_t i = 0; i < 5; ++i) {
            main_loop();
        }

        mm::TriggerStallGuard(mm::Selector);
        mm::TriggerStallGuard(mm::Idler);
        main_loop();
        mm::motion.StallGuardReset(mm::Selector);
        mm::motion.StallGuardReset(mm::Idler);
    }
    // now do a correct amount of steps of each axis towards the other end
    uint32_t idlerSteps = mm::unitToSteps<mm::I_pos_t>(config::idlerLimits.lenght);
    // now do LESS steps than expected to simulate something is blocking the selector
    uint32_t selectorSteps = mm::unitToSteps<mm::S_pos_t>(config::selectorLimits.lenght) + 1;
    uint32_t selectorTriggerShort = std::min(idlerSteps, selectorSteps) / 2;
    uint32_t maxSteps = selectorTriggerShort + 1;
    {
        for (uint32_t i = 0; i < maxSteps; ++i) {
            main_loop();

            if (i == selectorTriggerShort) {
                mm::TriggerStallGuard(mm::Selector);
            } else {
                mm::motion.StallGuardReset(mm::Selector);
            }
        }

        // make sure the Idler finishes its homing procedure (makes further checks much easier)
        for (uint32_t i = maxSteps; i < idlerSteps + 1; ++i) {
            main_loop();
            if (i == idlerSteps) {
                mm::TriggerStallGuard(mm::Idler);
            } else {
                mm::motion.StallGuardReset(mm::Idler);
            }
        }

        while (ms::selector.State() != mm::MovableBase::HomingFailed)
            main_loop();
    }

    return SimulateFailedHomePostfix(h);
}

template <typename T>
bool SimulateFailedHomeSelectorRepeated(T &h) {
    // we leave Idler aside in this case
    REQUIRE_FALSE(ms::selector.HomingValid());

    {
        // do 5 steps until we trigger the simulated stallguard
        for (uint8_t i = 0; i < 5; ++i) {
            main_loop();
        }

        mm::TriggerStallGuard(mm::Selector);
        main_loop();
        mm::motion.StallGuardReset(mm::Selector);
    }
    uint32_t selectorSteps = mm::unitToSteps<mm::S_pos_t>(config::selectorLimits.lenght) + 1;
    uint32_t selectorTriggerShort = selectorSteps / 2;
    uint32_t maxSteps = selectorTriggerShort + 1;
    {
        for (uint32_t i = 0; i < maxSteps; ++i) {
            main_loop();

            if (i == selectorTriggerShort) {
                mm::TriggerStallGuard(mm::Selector);
            } else {
                mm::motion.StallGuardReset(mm::Selector);
            }
        }

        while (ms::selector.State() != mm::MovableBase::HomingFailed)
            main_loop();
    }

    return SimulateFailedHomePostfix(h);
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

    SimulateSelectorHoming();

    REQUIRE(WhileTopState(h, ProgressCode::Homing, 5000));

    REQUIRE(VerifyState(h, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), 0, false, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));
    REQUIRE(mi::idler.HomingValid());
    REQUIRE(ms::selector.HomingValid());

    return true;
}

TEST_CASE("homing::selector_failed_retry", "[homing]") {
    REQUIRE(SelectorFailedRetry());
}
