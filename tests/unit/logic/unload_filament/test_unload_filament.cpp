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

namespace mm = modules::motion;
namespace mf = modules::finda;
namespace mi = modules::idler;
namespace ml = modules::leds;
namespace mb = modules::buttons;
namespace mg = modules::globals;
namespace ms = modules::selector;

bool VerifyState(logic::UnloadFilament &uf, bool filamentLoaded, uint8_t idlerSlotIndex, uint8_t selectorSlotIndex,
    bool findaPressed, ml::Mode greenLEDMode, ml::Mode redLEDMode, ErrorCode err, ProgressCode topLevelProgress) {
    CHECKED_ELSE(mg::globals.FilamentLoaded() == filamentLoaded) { return false; }
    CHECKED_ELSE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(idlerSlotIndex)) { return false; }
    CHECKED_ELSE(mi::idler.Engaged() == (idlerSlotIndex < 5)) { return false; }
    CHECKED_ELSE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(selectorSlotIndex)) { return false; }
    CHECKED_ELSE(ms::selector.Slot() == selectorSlotIndex) { return false; }
    CHECKED_ELSE(mf::finda.Pressed() == findaPressed) { return false; }
    CHECKED_ELSE(ml::leds.Mode(selectorSlotIndex, ml::red) == redLEDMode) { return false; }
    CHECKED_ELSE(ml::leds.Mode(selectorSlotIndex, ml::green) == greenLEDMode) { return false; }
    CHECKED_ELSE(uf.Error() == err) { return false; }
    CHECKED_ELSE(uf.TopLevelState() == topLevelProgress) { return false; }
    return true;
}

void RegularUnloadFromSlot04Init(uint8_t slot, logic::UnloadFilament &uf) {
    // prepare startup conditions
    ForceReinitAllAutomata();

    // change the startup to what we need here
    EnsureActiveSlotIndex(slot);

    mg::globals.SetFilamentLoaded(true);

    // set FINDA ON + debounce
    SetFINDAStateAndDebounce(true);

    // verify startup conditions
    REQUIRE(VerifyState(uf, true, 5, slot, true, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));

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
    REQUIRE(VerifyState(uf, true, 5, slot, true, ml::blink0, ml::off, ErrorCode::OK, ProgressCode::UnloadingToFinda));

    // run the automaton
    // Stage 1 - unloading to FINDA
    REQUIRE(WhileCondition(
        uf,
        [&](int step) -> bool {
        if(step == 100){ // on 100th step make FINDA trigger
            hal::adc::SetADC(1, 0);
        }
        return uf.TopLevelState() == ProgressCode::UnloadingToFinda; },
        5000));

    // we still think we have filament loaded at this stage
    // idler should have been activated by the underlying automaton
    // no change in selector's position
    // FINDA triggered off
    // green LED should blink
    REQUIRE(VerifyState(uf, true, slot, slot, false, ml::blink0, ml::off, ErrorCode::OK, ProgressCode::DisengagingIdler));

    // Stage 2 - idler was engaged, disengage it
    REQUIRE(WhileTopState(uf, ProgressCode::DisengagingIdler, 5000));

    // we still think we have filament loaded at this stage
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still triggered off
    // green LED should blink
    REQUIRE(VerifyState(uf, true, 5, slot, false, ml::blink0, ml::off, ErrorCode::OK, ProgressCode::AvoidingGrind));

    // Stage 3 - avoiding grind (whatever is that @@TODO)
    REQUIRE(WhileTopState(uf, ProgressCode::AvoidingGrind, 5000));

    // we still think we have filament loaded at this stage
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still triggered off
    // green LED should blink
    REQUIRE(VerifyState(uf, true, 5, slot, false, ml::blink0, ml::off, ErrorCode::OK, ProgressCode::FinishingMoves));

    // Stage 4 - finishing moves and setting global state correctly
    REQUIRE(WhileTopState(uf, ProgressCode::FinishingMoves, 5000));

    // filament unloaded
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still triggered off
    // green LED should be ON
    REQUIRE(VerifyState(uf, false, 5, slot, false, ml::on, ml::off, ErrorCode::OK, ProgressCode::OK));

    // Stage 5 - repeated calls to TopLevelState should return "OK"
    REQUIRE(uf.TopLevelState() == ProgressCode::OK);
    REQUIRE(mg::globals.FilamentLoaded() == false);
    REQUIRE(mf::finda.Pressed() == false);
    REQUIRE(uf.Error() == ErrorCode::OK); // no error
}

TEST_CASE("unload_filament::regular_unload_from_slot_0-4", "[unload_filament]") {
    for (uint8_t slot = 0; slot < 5; ++slot) {
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
    REQUIRE(VerifyState(uf, true, 5, slot, true, ml::off, ml::off, ErrorCode::OK, ProgressCode::OK));

    // restart the automaton
    uf.Reset(slot);

    // Stage 0 - verify state just after Reset()
    // we still think we have filament loaded at this stage
    // idler should have been activated by the underlying automaton
    // no change in selector's position
    // FINDA triggered off
    // green LED should blink
    // no error so far
    REQUIRE(VerifyState(uf, true, 5, slot, true, ml::blink0, ml::off, ErrorCode::OK, ProgressCode::UnloadingToFinda));

    // run the automaton
    // Stage 1 - unloading to FINDA - do NOT let it trigger - keep it pressed, the automaton should finish all moves with the pulley
    // without reaching the FINDA and report an error
    REQUIRE(WhileTopState(uf, ProgressCode::UnloadingToFinda, 50000));

    // we still think we have filament loaded at this stage
    // idler should have been activated by the underlying automaton
    // no change in selector's position
    // FINDA still on
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, slot, slot, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_TRIGGER, ProgressCode::ERR1DisengagingIdler));

    // Stage 2 - idler should get disengaged
    REQUIRE(WhileTopState(uf, ProgressCode::ERR1DisengagingIdler, 5000));

    // we still think we have filament loaded at this stage
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still on
    // red LED should blink
    // green LED should be off
    REQUIRE(VerifyState(uf, true, 5, slot, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_TRIGGER, ProgressCode::ERR1WaitingForUser));
}

void FindaDidntTriggerResolveHelp(uint8_t slot, logic::UnloadFilament &uf) {

    // Stage 3 - the user has to do something
    // there are 3 options:
    // - help the filament a bit
    // - try again the whole sequence
    // - resolve the problem by hand - after pressing the button we shall check, that FINDA is off and we should do what?

    // In this case we check the first option

    // Perform press on button 1 + debounce
    hal::adc::SetADC(0, 0);
    while (!mb::buttons.ButtonPressed(0)) {
        main_loop();
        uf.Step();
    }

    // we still think we have filament loaded at this stage
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still on
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, 5, slot, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_TRIGGER, ProgressCode::ERR1EngagingIdler));

    // Stage 4 - engage the idler
    REQUIRE(WhileTopState(uf, ProgressCode::ERR1EngagingIdler, 5000));

    // we still think we have filament loaded at this stage
    // idler should be engaged
    // no change in selector's position
    // FINDA still on
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, slot, slot, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_TRIGGER, ProgressCode::ERR1HelpingFilament));
}

void FindaDidntTriggerResolveHelpFindaTriggered(uint8_t slot, logic::UnloadFilament &uf) {
    // Stage 5 - move the pulley a bit - simulate FINDA depress
    REQUIRE(WhileCondition(
        uf,
        [&](int step) -> bool {
        if(step == 100){ // on 100th step make FINDA trigger
            hal::adc::SetADC(1, 0);
        }
        return uf.TopLevelState() == ProgressCode::ERR1HelpingFilament; },
        5000));

    // we still think we have filament loaded at this stage
    // idler should be engaged
    // no change in selector's position
    // FINDA depressed
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, slot, slot, false, ml::off, ml::blink0, ErrorCode::OK, ProgressCode::DisengagingIdler));
}

void FindaDidntTriggerResolveHelpFindaDidntTrigger(uint8_t slot, logic::UnloadFilament &uf) {
    // Stage 5 - move the pulley a bit - no FINDA change
    REQUIRE(WhileTopState(uf, ProgressCode::ERR1HelpingFilament, 5000));

    // we still think we have filament loaded at this stage
    // idler should be engaged
    // no change in selector's position
    // FINDA still pressed
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, slot, slot, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_TRIGGER, ProgressCode::ERR1DisengagingIdler));
}

TEST_CASE("unload_filament::finda_didnt_trigger_resolve_help", "[unload_filament]") {
    for (uint8_t slot = 0; slot < 5; ++slot) {
        logic::UnloadFilament uf;
        FindaDidntTriggerCommonSetup(slot, uf);
        FindaDidntTriggerResolveHelp(slot, uf);
        FindaDidntTriggerResolveHelpFindaTriggered(slot, uf);
    }

    // the same with different end scenario
    for (uint8_t slot = 0; slot < 5; ++slot) {
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
    hal::adc::SetADC(0, 340);
    while (!mb::buttons.ButtonPressed(1)) {
        main_loop();
        uf.Step();
    }

    // we still think we have filament loaded at this stage
    // idler should have been disengaged
    // no change in selector's position
    // FINDA still on
    // red LED should blink, green LED should be off
    REQUIRE(VerifyState(uf, true, 5, slot, true, ml::blink0, ml::off, ErrorCode::OK, ProgressCode::UnloadingToFinda));
}

TEST_CASE("unload_filament::finda_didnt_trigger_resolve_try_again", "[unload_filament]") {
    for (uint8_t slot = 0; slot < 5; ++slot) {
        logic::UnloadFilament uf;
        FindaDidntTriggerCommonSetup(slot, uf);
        FindaDidntTriggerResolveTryAgain(slot, uf);
        RegularUnloadFromSlot04(slot, uf);
    }
}
