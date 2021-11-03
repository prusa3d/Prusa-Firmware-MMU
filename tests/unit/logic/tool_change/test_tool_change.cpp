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

#include "../../../../src/logic/tool_change.h"

#include "../../modules/stubs/stub_adc.h"

#include "../stubs/main_loop_stub.h"
#include "../stubs/stub_motion.h"

using Catch::Matchers::Equals;

#include "../helpers/helpers.ipp"

void FeedingToFinda(logic::ToolChange &tc, uint8_t toSlot) {
    // feeding to finda
    REQUIRE(WhileCondition(
        tc,
        [&](int step) -> bool {
        if(step == 1000){ // on 1000th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::high);
        }
        return tc.TopLevelState() == ProgressCode::FeedingToFinda; },
        200000UL));
    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InSelector, toSlot, toSlot, true, true, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::FeedingToBondtech));
}

void FeedingToBondtech(logic::ToolChange &tc, uint8_t toSlot) {
    // james is feeding
    REQUIRE(WhileCondition(
        tc,
        [&](int step) -> bool {
        if(step == 2000){ // on 2000th step make filament sensor trigger
            mfs::fsensor.ProcessMessage(true);
        }
        return tc.TopLevelState() == ProgressCode::FeedingToBondtech; },
        20000UL));
    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InNozzle, mi::Idler::IdleSlotIndex(), toSlot, true, false, ml::on, ml::off, ErrorCode::OK, ProgressCode::OK));
}

void ToolChange(logic::ToolChange tc, uint8_t fromSlot, uint8_t toSlot) {
    ForceReinitAllAutomata();

    EnsureActiveSlotIndex(fromSlot, mg::FilamentLoadState::InNozzle);

    // restart the automaton
    tc.Reset(toSlot);

    REQUIRE(WhileCondition(
        tc,
        [&](int step) -> bool {
        if(step == 2000){ // on 2000th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::low);
        }
        return tc.TopLevelState() == ProgressCode::UnloadingFilament; },
        200000UL));
    CHECKED_ELSE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::AtPulley) {
        ++toSlot;
    }

    FeedingToFinda(tc, toSlot);

    FeedingToBondtech(tc, toSlot);

    REQUIRE(tc.TopLevelState() == ProgressCode::OK);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InNozzle);
    REQUIRE(mg::globals.ActiveSlot() == toSlot);
}

void NoToolChange(logic::ToolChange tc, uint8_t fromSlot, uint8_t toSlot) {
    ForceReinitAllAutomata();

    // the filament is LOADED
    EnsureActiveSlotIndex(fromSlot, mg::FilamentLoadState::InNozzle);

    REQUIRE(VerifyEnvironmentState(mg::FilamentLoadState::InNozzle, mi::Idler::IdleSlotIndex(), toSlot, false, false, ml::off, ml::off));

    // restart the automaton
    tc.Reset(toSlot);

    // should not do anything
    REQUIRE(tc.TopLevelState() == ProgressCode::OK);
    REQUIRE(tc.Error() == ErrorCode::OK);
}

void JustLoadFilament(logic::ToolChange tc, uint8_t slot) {
    ForceReinitAllAutomata();

    EnsureActiveSlotIndex(slot, mg::FilamentLoadState::AtPulley);

    // verify filament NOT loaded
    REQUIRE(VerifyEnvironmentState(mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), slot, false, false, ml::off, ml::off));

    // restart the automaton
    tc.Reset(slot);

    FeedingToFinda(tc, slot);

    FeedingToBondtech(tc, slot);

    REQUIRE(tc.TopLevelState() == ProgressCode::OK);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InNozzle);
    REQUIRE(mg::globals.ActiveSlot() == slot);
}

TEST_CASE("tool_change::test0", "[tool_change]") {
    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount; ++toSlot) {
            logic::ToolChange tc;
            if (fromSlot != toSlot) {
                ToolChange(tc, fromSlot, toSlot);
            } else {
                NoToolChange(tc, fromSlot, toSlot);
            }
        }
    }
}

TEST_CASE("tool_change::invalid_slot", "[tool_change]") {
    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        logic::ToolChange tc;
        InvalidSlot<logic::ToolChange>(tc, fromSlot, config::toolCount);
    }
}

TEST_CASE("tool_change::state_machine_reusal", "[tool_change]") {
    logic::ToolChange tc;

    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount + 2; ++toSlot) {
            if (toSlot >= config::toolCount) {
                InvalidSlot<logic::ToolChange>(tc, fromSlot, toSlot);
            } else if (fromSlot != toSlot) {
                ToolChange(tc, fromSlot, toSlot);
            } else {
                NoToolChange(tc, fromSlot, toSlot);
            }
        }
    }
}

TEST_CASE("tool_change::same_slot_just_unloaded_filament", "[tool_change]") {
    for (uint8_t toSlot = 0; toSlot < config::toolCount; ++toSlot) {
        logic::ToolChange tc;
        JustLoadFilament(tc, toSlot);
    }
}
