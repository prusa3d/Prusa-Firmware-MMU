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

namespace mm = modules::motion;
namespace mf = modules::finda;
namespace mi = modules::idler;
namespace ml = modules::leds;
namespace mb = modules::buttons;
namespace mg = modules::globals;
namespace ms = modules::selector;

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
            hal::adc::SetADC(1, 0);
        }
        return tc.TopLevelState() == ProgressCode::UnloadingFilament; },
        50000));
    REQUIRE(mg::globals.FilamentLoaded() == false);

    REQUIRE(WhileCondition(
        tc,
        [&](int step) -> bool {
        if(step == 1000){ // on 1000th step make FINDA trigger
            hal::adc::SetADC(1, 900);
        }
        return tc.TopLevelState() == ProgressCode::LoadingFilament; },
        50000));

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
    for (uint8_t fromSlot = 0; fromSlot < 5; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < 5; ++toSlot) {
            if (fromSlot != toSlot) {
                ToolChange(fromSlot, toSlot);
            } else {
                NoToolChange(fromSlot, toSlot);
            }
        }
    }
}
