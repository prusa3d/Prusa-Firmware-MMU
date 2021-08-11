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

void ToolChange(uint8_t fromSlot, uint8_t toSlot) {
    ForceReinitAllAutomata();

    logic::ToolChange tc;

    EnsureActiveSlotIndex(fromSlot);

    // restart the automaton
    tc.Reset(toSlot);

    REQUIRE(WhileCondition(
        tc,
        [&](int step) -> bool {
        if(step == 2000){ // on 2000th step make FINDA trigger
            hal::adc::SetADC(config::findaADCIndex, 0);
        }
        return tc.TopLevelState() == ProgressCode::UnloadingFilament; },
        200000UL));
    REQUIRE(mg::globals.FilamentLoaded() == false);

    REQUIRE(WhileCondition(
        tc,
        [&](int step) -> bool {
        if(step == 1000){ // on 1000th step make FINDA trigger
            hal::adc::SetADC(config::findaADCIndex, 900);
        }
        return tc.TopLevelState() == ProgressCode::LoadingFilament; },
        200000UL));

    REQUIRE(tc.TopLevelState() == ProgressCode::OK);
    REQUIRE(mg::globals.FilamentLoaded() == true);
    REQUIRE(mg::globals.ActiveSlot() == toSlot);
}

void NoToolChange(uint8_t fromSlot, uint8_t toSlot) {
    ForceReinitAllAutomata();

    logic::ToolChange tc;

    EnsureActiveSlotIndex(fromSlot);

    // restart the automaton
    tc.Reset(toSlot);

    // should not do anything
    REQUIRE(tc.TopLevelState() == ProgressCode::OK);
    REQUIRE(tc.Error() == ErrorCode::OK);
}

TEST_CASE("tool_change::test0", "[tool_change]") {
    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount; ++toSlot) {
            if (fromSlot != toSlot) {
                ToolChange(fromSlot, toSlot);
            } else {
                NoToolChange(fromSlot, toSlot);
            }
        }
    }
}

TEST_CASE("tool_change::invalid_slot", "[tool_change]") {
    for (uint8_t cutSlot = 0; cutSlot < config::toolCount; ++cutSlot) {
        InvalidSlot<logic::ToolChange>(config::toolCount, cutSlot);
    }
}
