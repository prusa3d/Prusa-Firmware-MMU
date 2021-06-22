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

void RegularUnloadFromSlot04(uint8_t slot) {
    // prepare startup conditions
    ForceReinitAllAutomata();

    // change the startup to what we need here
    // move selector to the right spot
    ms::selector.MoveToSlot(slot);
    while (ms::selector.Slot() != slot)
        main_loop();

    mg::globals.SetActiveSlot(slot);
    mg::globals.SetFilamentLoaded(true);

    // set FINDA ON + debounce
    hal::adc::SetADC(1, mf::FINDA::adcDecisionLevel + 1);
    for (size_t i = 0; i < mf::FINDA::debounce + 1; ++i)
        main_loop();

    // verify startup conditions
    REQUIRE(mg::globals.FilamentLoaded() == true);
    REQUIRE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5));
    REQUIRE(!mi::idler.Engaged());
    REQUIRE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(slot));
    REQUIRE(ms::selector.Slot() == slot);
    REQUIRE(mf::finda.Pressed() == true);

    // restart the automaton
    logic::UnloadFilament uf;
    uf.Reset(slot);

    // Stage 0 - verify state just after Reset()
    REQUIRE(mg::globals.FilamentLoaded() == true); // we still think we have filament loaded at this stage
    REQUIRE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5)); // idler should have been activated by the underlying automaton
    REQUIRE(!mi::idler.Engaged());
    REQUIRE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(slot)); // no change in selector's position
    REQUIRE(ms::selector.Slot() == slot);
    REQUIRE(mf::finda.Pressed() == true); // FINDA triggered off
    REQUIRE(ml::leds.Mode(slot, ml::green) == ml::blink0); // green LED should blink
    REQUIRE(uf.Error() == ErrorCode::OK); // no error so far

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

    REQUIRE(mg::globals.FilamentLoaded() == true); // we still think we have filament loaded at this stage
    REQUIRE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(slot)); // idler should have been activated by the underlying automaton
    REQUIRE(mi::idler.Engaged());
    REQUIRE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(slot)); // no change in selector's position
    REQUIRE(ms::selector.Slot() == slot);
    REQUIRE(mf::finda.Pressed() == false); // FINDA triggered off
    REQUIRE(ml::leds.Mode(slot, ml::green) == ml::blink0); // green LED should blink
    REQUIRE(uf.Error() == ErrorCode::OK); // no error so far

    // Stage 2 - idler was engaged, disengage it
    REQUIRE(uf.TopLevelState() == ProgressCode::DisengagingIdler);
    REQUIRE(WhileTopState(uf, ProgressCode::DisengagingIdler, 5000));

    REQUIRE(mg::globals.FilamentLoaded() == true); // we still think we have filament loaded at this stage
    REQUIRE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5)); // idler should have been disengaged
    REQUIRE(!mi::idler.Engaged());
    REQUIRE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(slot)); // no change in selector's position
    REQUIRE(ms::selector.Slot() == slot);
    REQUIRE(mf::finda.Pressed() == false); // FINDA still triggered off
    REQUIRE(ml::leds.Mode(slot, ml::green) == ml::blink0); // green LED should blink
    REQUIRE(uf.Error() == ErrorCode::OK); // no error so far

    // Stage 3 - avoiding grind (whatever is that @@TODO)
    REQUIRE(uf.TopLevelState() == ProgressCode::AvoidingGrind);
    REQUIRE(WhileTopState(uf, ProgressCode::AvoidingGrind, 5000));

    REQUIRE(mg::globals.FilamentLoaded() == true); // we still think we have filament loaded at this stage
    REQUIRE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5)); // idler should have been disengaged
    REQUIRE(!mi::idler.Engaged());
    REQUIRE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(slot)); // no change in selector's position
    REQUIRE(ms::selector.Slot() == slot);
    REQUIRE(mf::finda.Pressed() == false); // FINDA still triggered off
    REQUIRE(ml::leds.Mode(slot, ml::green) == ml::blink0); // green LED should blink
    REQUIRE(uf.Error() == ErrorCode::OK); // no error so far

    // Stage 4 - finishing moves and setting global state correctly
    REQUIRE(uf.TopLevelState() == ProgressCode::FinishingMoves);
    REQUIRE(WhileTopState(uf, ProgressCode::FinishingMoves, 5000));

    REQUIRE(mg::globals.FilamentLoaded() == false); // filament unloaded
    REQUIRE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5)); // idler should have been disengaged
    REQUIRE(!mi::idler.Engaged());
    REQUIRE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(slot)); // no change in selector's position
    REQUIRE(ms::selector.Slot() == slot);
    REQUIRE(mf::finda.Pressed() == false); // FINDA still triggered off
    REQUIRE(ml::leds.Mode(slot, ml::green) == ml::on); // green LED should be ON
    REQUIRE(uf.Error() == ErrorCode::OK); // no error so far

    // Stage 5 - repeated calls to TopLevelState should return "OK"
    REQUIRE(uf.TopLevelState() == ProgressCode::OK);
    REQUIRE(mg::globals.FilamentLoaded() == false);
    REQUIRE(mf::finda.Pressed() == false);
    REQUIRE(uf.Error() == ErrorCode::OK); // no error
}

TEST_CASE("unload_filament::regular_unload_from_slot_0-4", "[unload_filament]") {
    for (uint8_t slot = 0; slot < 5; ++slot) {
        RegularUnloadFromSlot04(slot);
    }
}

void FindaDidntTrigger(uint8_t slot) {
    // prepare startup conditions
    ForceReinitAllAutomata();

    // change the startup to what we need here

    // move selector to the right spot
    ms::selector.MoveToSlot(slot);
    while (ms::selector.Slot() != slot)
        main_loop();

    // set FINDA ON + debounce
    hal::adc::SetADC(1, mf::FINDA::adcDecisionLevel + 1);
    for (size_t i = 0; i < mf::FINDA::debounce + 1; ++i)
        main_loop();

    mg::globals.SetActiveSlot(slot);
    mg::globals.SetFilamentLoaded(true);

    // verify startup conditions
    REQUIRE(mg::globals.FilamentLoaded() == true);
    REQUIRE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5));
    REQUIRE(!mi::idler.Engaged());
    REQUIRE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(slot));
    REQUIRE(ms::selector.Slot() == slot);
    REQUIRE(mf::finda.Pressed() == true);

    // restart the automaton
    logic::UnloadFilament uf;
    uf.Reset(slot);

    // Stage 0 - verify state just after Reset()
    REQUIRE(mg::globals.FilamentLoaded() == true); // we still think we have filament loaded at this stage
    REQUIRE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5)); // idler should have been activated by the underlying automaton
    REQUIRE(!mi::idler.Engaged());
    REQUIRE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(slot)); // no change in selector's position
    REQUIRE(ms::selector.Slot() == slot);
    REQUIRE(mf::finda.Pressed() == true); // FINDA triggered off
    REQUIRE(ml::leds.Mode(slot, ml::green) == ml::blink0); // green LED should blink
    REQUIRE(uf.Error() == ErrorCode::OK); // no error so far

    // run the automaton
    // Stage 1 - unloading to FINDA - do NOT let it trigger - keep it pressed, the automaton should finish all moves with the pulley
    // without reaching the FINDA and report an error
    REQUIRE(WhileTopState(uf, ProgressCode::UnloadingToFinda, 50000));

    REQUIRE(mg::globals.FilamentLoaded() == true); // we still think we have filament loaded at this stage
    REQUIRE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(slot)); // idler should have been activated by the underlying automaton
    REQUIRE(mi::idler.Engaged());
    REQUIRE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(slot)); // no change in selector's position
    REQUIRE(ms::selector.Slot() == slot);
    REQUIRE(mf::finda.Pressed() == true); // FINDA still on
    REQUIRE(ml::leds.Mode(slot, ml::red) == ml::blink0); // red LED should blink
    REQUIRE(ml::leds.Mode(slot, ml::green) == ml::off); // green LED should be off
    REQUIRE(uf.Error() == ErrorCode::FINDA_DIDNT_TRIGGER); // didn't get any response from FINDA
    REQUIRE(uf.TopLevelState() == ProgressCode::ERR1DisengagingIdler);

    // Stage 2 - idler should get disengaged
    REQUIRE(WhileTopState(uf, ProgressCode::ERR1DisengagingIdler, 5000));

    REQUIRE(mg::globals.FilamentLoaded() == true); // we still think we have filament loaded at this stage
    REQUIRE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5)); // idler should have been disengaged
    REQUIRE(!mi::idler.Engaged());
    REQUIRE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(slot)); // no change in selector's position
    REQUIRE(ms::selector.Slot() == slot);
    REQUIRE(mf::finda.Pressed() == true); // FINDA still on
    REQUIRE(ml::leds.Mode(slot, ml::red) == ml::blink0); // red LED should blink
    REQUIRE(ml::leds.Mode(slot, ml::green) == ml::off); // green LED should be off
    REQUIRE(uf.Error() == ErrorCode::FINDA_DIDNT_TRIGGER);
    REQUIRE(uf.TopLevelState() == ProgressCode::ERR1WaitingForUser);

    // Stage 3 - the user has to do something
    // there are 3 options:
    // - help the filament a bit
    // - try again the whole sequence
    // - resolve the problem by hand - after pressing the button we shall check, that FINDA is off and we should do what?
}

TEST_CASE("unload_filament::finda_didnt_trigger", "[unload_filament]") {
    for (uint8_t slot = 0; slot < 5; ++slot) {
        FindaDidntTrigger(slot);
    }
}
