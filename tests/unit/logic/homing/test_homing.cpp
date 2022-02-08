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
