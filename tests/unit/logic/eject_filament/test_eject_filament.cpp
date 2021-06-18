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

#include "../../../../src/logic/eject_filament.h"

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

TEST_CASE("eject_filament::eject0", "[eject_filament]") {
    using namespace logic;

    ForceReinitAllAutomata();

    EjectFilament ef;
    // restart the automaton
    currentCommand = &ef;
    ef.Reset(0);

    main_loop();

    // it should have instructed the selector and idler to move to slot 1
    // check if the idler and selector have the right command
    CHECK(modules::motion::axes[modules::motion::Idler].targetPos == mi::Idler::SlotPosition(0));
    CHECK(modules::motion::axes[modules::motion::Selector].targetPos == ms::Selector::SlotPosition(4));

    // now cycle at most some number of cycles (to be determined yet) and then verify, that the idler and selector reached their target positions
    REQUIRE(WhileCondition([&]() { return ef.TopLevelState() == ProgressCode::SelectingFilamentSlot; }, 5000));

    // idler and selector reached their target positions and the CF automaton will start feeding to FINDA as the next step
    REQUIRE(ef.TopLevelState() == ProgressCode::FeedingToFinda);
    // prepare for simulated finda trigger
    hal::adc::ReinitADC(1, hal::adc::TADCData({ 0, 0, 0, 0, 600, 700, 800, 900 }), 10);
    REQUIRE(WhileCondition([&]() { return ef.TopLevelState() == ProgressCode::FeedingToFinda; }, 50000));

    // filament fed into FINDA, cutting...
    REQUIRE(ef.TopLevelState() == ProgressCode::PreparingBlade);
    REQUIRE(WhileCondition([&]() { return ef.TopLevelState() == ProgressCode::PreparingBlade; }, 5000));

    REQUIRE(ef.TopLevelState() == ProgressCode::EngagingIdler);
    REQUIRE(WhileCondition([&]() { return ef.TopLevelState() == ProgressCode::EngagingIdler; }, 5000));

    // the idler should be at the active slot @@TODO
    REQUIRE(ef.TopLevelState() == ProgressCode::PushingFilament);
    REQUIRE(WhileCondition([&]() { return ef.TopLevelState() == ProgressCode::PushingFilament; }, 5000));

    // filament pushed - performing cut
    REQUIRE(ef.TopLevelState() == ProgressCode::PerformingCut);
    REQUIRE(WhileCondition([&]() { return ef.TopLevelState() == ProgressCode::PerformingCut; }, 5000));

    // returning selector
    REQUIRE(ef.TopLevelState() == ProgressCode::ReturningSelector);
    REQUIRE(WhileCondition([&]() { return ef.TopLevelState() == ProgressCode::ReturningSelector; }, 5000));

    // the next states are still @@TODO
}

// comments:
// The tricky part of the whole state machine are the edge cases - filament not loaded, stall guards etc.
// ... all the external influence we can get on the real HW
// But the good news is we can simulate them all in the unit test and thus ensure proper handling
