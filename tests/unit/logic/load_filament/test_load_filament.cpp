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

#include "../../../../src/logic/load_filament.h"

#include "../../modules/stubs/stub_adc.h"

#include "../stubs/main_loop_stub.h"
#include "../stubs/stub_motion.h"

using Catch::Matchers::Equals;

#include "../helpers/helpers.ipp"

void LoadFilamentCommonSetup(uint8_t slot, logic::LoadFilament &lf) {
    ForceReinitAllAutomata();

    // change the startup to what we need here
    EnsureActiveSlotIndex(slot, mg::FilamentLoadState::AtPulley);

    // verify startup conditions
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), slot, false, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));

    // restart the automaton
    lf.Reset(slot);

    // Stage 0 - verify state just after Reset()
    // we assume the filament is not loaded
    // idler should have been activated by the underlying automaton
    // no change in selector's position
    // FINDA off
    // green LED should blink, red off
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), slot, false, false, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::FeedingToFinda));
}

void LoadFilamentSuccessful(uint8_t slot, logic::LoadFilament &lf) {
    // Stage 2 - feeding to finda
    // we'll assume the finda is working correctly here
    REQUIRE(WhileCondition(
        lf,
        [&](int step) -> bool {
        if(step == 100){ // on 100th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::high);
        }
        return lf.TopLevelState() == ProgressCode::FeedingToFinda; },
        5000));
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::InSelector, slot, slot, true, true, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::RetractingFromFinda));

    // Stage 3 - retracting from finda
    // we'll assume the finda is working correctly here
    REQUIRE(WhileCondition(
        lf,
        [&](int step) -> bool {
        if(step == 50){ // on 50th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::low);
        }
        return lf.TopLevelState() == ProgressCode::RetractingFromFinda; },
        5000));
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::AtPulley, slot, slot, false, true, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::DisengagingIdler));

    // Stage 4 - disengaging idler
    REQUIRE(WhileTopState(lf, ProgressCode::DisengagingIdler, idlerEngageDisengageMaxSteps));
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), slot, false, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));
}

TEST_CASE("load_filament::regular_load_to_slot_0-4", "[load_filament]") {
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        logic::LoadFilament lf;
        LoadFilamentCommonSetup(slot, lf);
        LoadFilamentSuccessful(slot, lf);
    }
}

void FailedLoadToFinda(uint8_t slot, logic::LoadFilament &lf) {
    // Stage 2 - feeding to finda
    // we'll assume the finda is defective here and does not trigger
    REQUIRE(WhileTopState(lf, ProgressCode::FeedingToFinda, 50000));
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::InSelector, slot, slot, false, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_ON, ProgressCode::ERRDisengagingIdler));

    // Stage 3 - disengaging idler in error mode
    REQUIRE(WhileTopState(lf, ProgressCode::ERRDisengagingIdler, idlerEngageDisengageMaxSteps));
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::InSelector, mi::Idler::IdleSlotIndex(), slot, false, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_ON, ProgressCode::ERRWaitingForUser));
}

void FailedLoadToFindaResolveHelp(uint8_t slot, logic::LoadFilament &lf) {
    // Stage 3 - the user has to do something
    // there are 3 options:
    // - help the filament a bit
    // - try again the whole sequence
    // - resolve the problem by hand - after pressing the button we shall check, that FINDA is off and we should do what?

    // In this case we check the first option

    // Perform press on button 0 + debounce
    hal::adc::SetADC(config::buttonsADCIndex, config::buttonADCLimits[0][0] + 1);
    while (!mb::buttons.ButtonPressed(0)) {
        main_loop();
        lf.Step();
    }

    REQUIRE(VerifyState(lf, mg::FilamentLoadState::InSelector, mi::Idler::IdleSlotIndex(), slot, false, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_ON, ProgressCode::ERREngagingIdler));

    // Stage 4 - engage the idler
    REQUIRE(WhileTopState(lf, ProgressCode::ERREngagingIdler, idlerEngageDisengageMaxSteps));

    REQUIRE(VerifyState(lf, mg::FilamentLoadState::InSelector, slot, slot, false, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_ON, ProgressCode::ERRHelpingFilament));
}

void FailedLoadToFindaResolveHelpFindaTriggered(uint8_t slot, logic::LoadFilament &lf) {
    // Stage 5 - move the pulley a bit - simulate FINDA depress
    REQUIRE(WhileCondition(
        lf,
        [&](int step) -> bool {
        if(step == 100){ // on 100th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::high);
        }
        return lf.TopLevelState() == ProgressCode::ERRHelpingFilament; },
        5000));

    REQUIRE(VerifyState(lf, mg::FilamentLoadState::InSelector, slot, slot, true, true, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::RetractingFromFinda));
}

void FailedLoadToFindaResolveHelpFindaDidntTrigger(uint8_t slot, logic::LoadFilament &lf) {
    // Stage 5 - move the pulley a bit - no FINDA change
    REQUIRE(WhileTopState(lf, ProgressCode::ERRHelpingFilament, 5000));

    REQUIRE(VerifyState(lf, mg::FilamentLoadState::InSelector, slot, slot, false, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_ON, ProgressCode::ERRDisengagingIdler));
}

void FailedLoadToFindaResolveManual(uint8_t slot, logic::LoadFilament &lf) {
    // simulate the user fixed the issue himself

    // Perform press on button 2 + debounce + switch on FINDA
    hal::adc::SetADC(config::buttonsADCIndex, config::buttonADCLimits[2][0] + 1);
    hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::high);
    while (!mb::buttons.ButtonPressed(2)) {
        main_loop();
        lf.Step();
    }
    // pulling filament back
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::InSelector, mi::Idler::IdleSlotIndex(), slot, true, true, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::RetractingFromFinda));

    // Stage 3 - retracting from finda
    // we'll assume the finda is working correctly here
    REQUIRE(WhileCondition(
        lf,
        [&](int step) -> bool {
        if(step == 50){ // on 50th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::low);
        }
        return lf.TopLevelState() == ProgressCode::RetractingFromFinda; },
        5000));
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::AtPulley, slot, slot, false, true, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::DisengagingIdler));

    // disengaging idler
    REQUIRE(WhileTopState(lf, ProgressCode::DisengagingIdler, idlerEngageDisengageMaxSteps));
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), slot, false, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));
}

void FailedLoadToFindaResolveManualNoFINDA(uint8_t slot, logic::LoadFilament &lf) {
    // Perform press on button 2 + debounce + keep FINDA OFF (i.e. the user didn't solve anything)
    hal::adc::SetADC(config::buttonsADCIndex, config::buttonADCLimits[2][0] + 1);
    while (!mb::buttons.ButtonPressed(2)) {
        main_loop();
        lf.Step();
    }
    // pulling filament back
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::InSelector, mi::Idler::IdleSlotIndex(), slot, false, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_ON, ProgressCode::ERRWaitingForUser));
}

void FailedLoadToFindaResolveTryAgain(uint8_t slot, logic::LoadFilament &lf) {
    // Perform press on button 1 + debounce
    hal::adc::SetADC(config::buttonsADCIndex, config::buttonADCLimits[1][0] + 1);
    while (!mb::buttons.ButtonPressed(1)) {
        main_loop();
        lf.Step();
    }

    // the state machine should have restarted
    REQUIRE(VerifyState(lf, mg::FilamentLoadState::InSelector, mi::Idler::IdleSlotIndex(), slot, false, true, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::FeedingToFinda));
    LoadFilamentSuccessful(slot, lf);
}

TEST_CASE("load_filament::failed_load_to_finda_0-4_resolve_help_second_ok", "[load_filament]") {
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        logic::LoadFilament lf;
        LoadFilamentCommonSetup(slot, lf);
        FailedLoadToFinda(slot, lf);
        FailedLoadToFindaResolveHelp(slot, lf);
        FailedLoadToFindaResolveHelpFindaTriggered(slot, lf);
    }
}

TEST_CASE("load_filament::failed_load_to_finda_0-4_resolve_help_second_fail", "[load_filament]") {
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        logic::LoadFilament lf;
        LoadFilamentCommonSetup(slot, lf);
        FailedLoadToFinda(slot, lf);
        FailedLoadToFindaResolveHelp(slot, lf);
        FailedLoadToFindaResolveHelpFindaDidntTrigger(slot, lf);
    }
}

TEST_CASE("load_filament::invalid_slot", "[load_filament]") {
    for (uint8_t activeSlot = 0; activeSlot < config::toolCount; ++activeSlot) {
        logic::LoadFilament lf;
        InvalidSlot<logic::LoadFilament>(lf, activeSlot, config::toolCount);
    }
}

TEST_CASE("load_filament::state_machine_reusal", "[load_filament]") {
    logic::LoadFilament lf;

    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount + 2; ++toSlot) {
            logic::LoadFilament lf;
            if (toSlot >= config::toolCount) {
                InvalidSlot<logic::LoadFilament>(lf, fromSlot, toSlot);
            } else {
                LoadFilamentCommonSetup(toSlot, lf);
                LoadFilamentSuccessful(toSlot, lf);
            }
        }
    }
}

TEST_CASE("load_filament::failed_load_to_finda_0-4_resolve_manual", "[load_filament]") {
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        logic::LoadFilament lf;
        LoadFilamentCommonSetup(slot, lf);
        FailedLoadToFinda(slot, lf);
        FailedLoadToFindaResolveManual(slot, lf);
    }
}

TEST_CASE("load_filament::failed_load_to_finda_0-4_resolve_manual_no_FINDA", "[load_filament]") {
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        logic::LoadFilament lf;
        LoadFilamentCommonSetup(slot, lf);
        FailedLoadToFinda(slot, lf);
        FailedLoadToFindaResolveManualNoFINDA(slot, lf);
    }
}

TEST_CASE("load_filament::failed_load_to_finda_0-4_try_again", "[load_filament]") {
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        logic::LoadFilament lf;
        LoadFilamentCommonSetup(slot, lf);
        FailedLoadToFinda(slot, lf);
        FailedLoadToFindaResolveTryAgain(slot, lf);
    }
}
