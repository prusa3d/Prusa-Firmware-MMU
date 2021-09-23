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

#include "../../../../src/logic/unload_filament.h"

#include "../../modules/stubs/stub_adc.h"

#include "../stubs/main_loop_stub.h"
#include "../stubs/stub_motion.h"

using Catch::Matchers::Equals;

#include "../helpers/helpers.ipp"

void RegularUnloadFromSlot04Init(uint8_t slot, logic::UnloadFilament &uf) {
    // prepare startup conditions
    ForceReinitAllAutomata();

    // change the startup to what we need here
    EnsureActiveSlotIndex(slot);

    mg::globals.SetFilamentLoaded(true);

    // set FINDA ON + debounce
    SetFINDAStateAndDebounce(true);

    // verify startup conditions
    REQUIRE(VerifyState(uf, true, mi::Idler::IdleSlotIndex(), slot, true, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));

    // restart the automaton
    uf.Reset(slot);
}

void RegularUnloadFromSlot04(uint8_t slot, logic::UnloadFilament &uf) {
    // Stage 0 - verify state just after Reset()
    // we still think we have filament loaded at this stage
    // idler should have been activated by the underlying automaton
    // no change in selector's position
    // FINDA on
    // green LED should blink, red off
    REQUIRE(VerifyState(uf, true, mi::Idler::IdleSlotIndex(), slot, true, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::UnloadingToFinda));

    // run the automaton
    // Stage 1 - unloading to FINDA
    REQUIRE(WhileCondition(
        uf,
        [&](int step) -> bool {
        if(step == 100){ // on 100th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::low);
        }
        return uf.TopLevelState() == ProgressCode::UnloadingToFinda; },
        50000));

    // we still think we have filament loaded at this stage
    // idler should have been activated by the underlying automaton
    // no change in selector's position
    // FINDA triggered off
    // green LED should be off
    REQUIRE(VerifyState(uf, true, slot, slot, false, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::DisengagingIdler));

    // Stage 2 - idler was engaged, disengage it
    REQUIRE(WhileTopState(uf, ProgressCode::DisengagingIdler, idlerEngageDisengageMaxSteps));

    // we still think we have filament loaded at this stage
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still triggered off
    // green LED should be off
    REQUIRE(VerifyState(uf, true, mi::Idler::IdleSlotIndex(), slot, false, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::AvoidingGrind));

    // Stage 3 - avoiding grind (whatever is that @@TODO)
    REQUIRE(WhileTopState(uf, ProgressCode::AvoidingGrind, 5000));

    // we still think we have filament loaded at this stage
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still triggered off
    // green LED should be off
    REQUIRE(VerifyState(uf, true, mi::Idler::IdleSlotIndex(), slot, false, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::FinishingMoves));

    // Stage 4 - finishing moves and setting global state correctly
    REQUIRE(WhileTopState(uf, ProgressCode::FinishingMoves, 5000));

    // filament unloaded
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still triggered off
    // green LED should be OFF
    REQUIRE(VerifyState(uf, false, mi::Idler::IdleSlotIndex(), slot, false, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));

    // Stage 5 - repeated calls to TopLevelState should return "OK"
    REQUIRE(uf.TopLevelState() == ProgressCode::OK);
    REQUIRE(mg::globals.FilamentLoaded() == false);
    REQUIRE(mf::finda.Pressed() == false);
    REQUIRE(uf.Error() == ErrorCode::OK); // no error
}

TEST_CASE("unload_filament::regular_unload_from_slot_0-4", "[unload_filament]") {
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        logic::UnloadFilament uf;
        RegularUnloadFromSlot04Init(slot, uf);
        RegularUnloadFromSlot04(slot, uf);
    }
}

void FindaDidntTriggerCommonSetup(uint8_t slot, logic::UnloadFilament &uf) {
    // prepare startup conditions
    ForceReinitAllAutomata();

    // change the startup to what we need here
    // move selector to the right spot
    EnsureActiveSlotIndex(slot);

    // set FINDA ON + debounce
    SetFINDAStateAndDebounce(true);

    mg::globals.SetFilamentLoaded(true);

    // verify startup conditions
    REQUIRE(VerifyState(uf, true, mi::Idler::IdleSlotIndex(), slot, true, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));

    // restart the automaton
    uf.Reset(slot);

    // Stage 0 - verify state just after Reset()
    // we still think we have filament loaded at this stage
    // idler should have been activated by the underlying automaton
    // no change in selector's position
    // FINDA triggered off
    // green LED should be off
    // no error so far
    REQUIRE(VerifyState(uf, true, mi::Idler::IdleSlotIndex(), slot, true, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::UnloadingToFinda));

    // run the automaton
    // Stage 1 - unloading to FINDA - do NOT let it trigger - keep it pressed, the automaton should finish all moves with the pulley
    // without reaching the FINDA and report an error
    REQUIRE(WhileTopState(uf, ProgressCode::UnloadingToFinda, 50000));

    // we still think we have filament loaded at this stage
    // idler should have been activated by the underlying automaton
    // no change in selector's position
    // FINDA still on
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, slot, slot, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_OFF, ProgressCode::ERRDisengagingIdler));

    // Stage 2 - idler should get disengaged
    REQUIRE(WhileTopState(uf, ProgressCode::ERRDisengagingIdler, idlerEngageDisengageMaxSteps));

    // we still think we have filament loaded at this stage
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still on
    // red LED should blink
    // green LED should be off
    REQUIRE(VerifyState(uf, true, mi::Idler::IdleSlotIndex(), slot, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_OFF, ProgressCode::ERRWaitingForUser));
}

void FindaDidntTriggerResolveHelp(uint8_t slot, logic::UnloadFilament &uf) {

    // Stage 3 - the user has to do something
    // there are 3 options:
    // - help the filament a bit
    // - try again the whole sequence
    // - resolve the problem by hand - after pressing the button we shall check, that FINDA is off and we should do what?

    // In this case we check the first option

    // Perform press on button 1 + debounce
    hal::adc::SetADC(config::buttonsADCIndex, config::buttonADCLimits[0][0] + 1);
    while (!mb::buttons.ButtonPressed(0)) {
        main_loop();
        uf.StepInner();
    }

    // we still think we have filament loaded at this stage
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still on
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, mi::Idler::IdleSlotIndex(), slot, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_OFF, ProgressCode::ERREngagingIdler));

    // Stage 4 - engage the idler
    REQUIRE(WhileTopState(uf, ProgressCode::ERREngagingIdler, idlerEngageDisengageMaxSteps));

    // we still think we have filament loaded at this stage
    // idler should be engaged
    // no change in selector's position
    // FINDA still on
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, slot, slot, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_OFF, ProgressCode::ERRHelpingFilament));
}

void FindaDidntTriggerResolveHelpFindaTriggered(uint8_t slot, logic::UnloadFilament &uf) {
    // Stage 5 - move the pulley a bit - simulate FINDA depress
    REQUIRE(WhileCondition(
        uf,
        [&](int step) -> bool {
        if(step == 100){ // on 100th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::low);
        }
        return uf.TopLevelState() == ProgressCode::ERRHelpingFilament; },
        5000));

    // we still think we have filament loaded at this stage
    // idler should be engaged
    // no change in selector's position
    // FINDA depressed
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, slot, slot, false, ml::off, ml::blink0, ErrorCode::RUNNING, ProgressCode::DisengagingIdler));
}

void FindaDidntTriggerResolveHelpFindaDidntTrigger(uint8_t slot, logic::UnloadFilament &uf) {
    // Stage 5 - move the pulley a bit - no FINDA change
    REQUIRE(WhileTopState(uf, ProgressCode::ERRHelpingFilament, 5000));

    // we still think we have filament loaded at this stage
    // idler should be engaged
    // no change in selector's position
    // FINDA still pressed
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, slot, slot, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_OFF, ProgressCode::ERRDisengagingIdler));
}

TEST_CASE("unload_filament::finda_didnt_trigger_resolve_help_second_ok", "[unload_filament]") {
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        logic::UnloadFilament uf;
        FindaDidntTriggerCommonSetup(slot, uf);
        FindaDidntTriggerResolveHelp(slot, uf);
        FindaDidntTriggerResolveHelpFindaTriggered(slot, uf);
    }
}

TEST_CASE("unload_filament::finda_didnt_trigger_resolve_help_second_fail", "[unload_filament]") {
    // the same with different end scenario
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        logic::UnloadFilament uf;
        FindaDidntTriggerCommonSetup(slot, uf);
        FindaDidntTriggerResolveHelp(slot, uf);
        FindaDidntTriggerResolveHelpFindaDidntTrigger(slot, uf);
    }
}

void FindaDidntTriggerResolveTryAgain(uint8_t slot, logic::UnloadFilament &uf) {
    // Stage 3 - the user has to do something
    // there are 3 options:
    // - help the filament a bit
    // - try again the whole sequence
    // - resolve the problem by hand - after pressing the button we shall check, that FINDA is off and we should do what?

    // In this case we check the second option

    // Perform press on button 2 + debounce
    hal::adc::SetADC(config::buttonsADCIndex, config::buttonADCLimits[1][0] + 1);
    while (!mb::buttons.ButtonPressed(1)) {
        main_loop();
        uf.StepInner();
    }

    // we still think we have filament loaded at this stage
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still on
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, mi::Idler::IdleSlotIndex(), slot, true, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::UnloadingToFinda));
}

TEST_CASE("unload_filament::finda_didnt_trigger_resolve_try_again", "[unload_filament]") {
    for (uint8_t slot = 0; slot < config::toolCount; ++slot) {
        logic::UnloadFilament uf;
        FindaDidntTriggerCommonSetup(slot, uf);
        FindaDidntTriggerResolveTryAgain(slot, uf);
        RegularUnloadFromSlot04(slot, uf);
    }
}
