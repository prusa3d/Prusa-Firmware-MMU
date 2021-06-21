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

#include "../../../../src/logic/feed_to_bondtech.h"

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

namespace ha = hal::adc;

template <typename COND>
bool WhileCondition(logic::FeedToBondtech &ff, COND cond, uint32_t maxLoops = 5000) {
    while (cond() && --maxLoops) {
        main_loop();
        ff.Step();
    }
    return maxLoops > 0;
}

TEST_CASE("feed_to_finda::feed_phase_unlimited", "[feed_to_finda]") {
    using namespace logic;

    ForceReinitAllAutomata();

    FeedToBondtech fb;
    main_loop();

    // restart the automaton
    fb.Reset(false);

    //    REQUIRE(ff.State() == FeedToBondtech::EngagingIdler);

    //    // it should have instructed the selector and idler to move to slot 1
    //    // check if the idler and selector have the right command
    //    CHECK(mm::axes[mm::Idler].targetPos == mi::Idler::SlotPosition(0));
    //    CHECK(mm::axes[mm::Selector].targetPos == ms::Selector::SlotPosition(0));
    //    CHECK(mm::axes[mm::Idler].enabled == true);

    //    // engaging idler
    //    REQUIRE(WhileCondition(
    //        ff,
    //        [&]() { return !mi::idler.Engaged(); },
    //        5000));

    //    CHECK(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(0));
    //    CHECK(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(0));

    //    // idler engaged, selector in position, we'll start pushing filament
    //    REQUIRE(ff.State() == FeedToBondtech::PushingFilament);
    //    // at least at the beginning the LED should shine green (it should be blinking, but this mode has been already verified in the LED's unit test)
    //    REQUIRE(ml::leds.LedOn(mg::globals.ActiveSlot(), ml::Color::green));

    //    // now let the filament be pushed into the FINDA - do 500 steps without triggering the condition
    //    // and then let the simulated ADC channel 1 create a FINDA switch
    //    ha::ReinitADC(1, ha::TADCData({ 600, 700, 800, 900 }), 1);

    //    REQUIRE(WhileCondition(
    //        ff,
    //        [&]() { return ff.State() == FeedToBondtech::PushingFilament; },
    //        1500));
    //    // From now on the FINDA is reported as ON

    //    // unloading back to PTFE
    //    REQUIRE(ff.State() == FeedToBondtech::UnloadBackToPTFE);
    //    REQUIRE(WhileCondition(
    //        ff,
    //        [&]() { return ff.State() == FeedToBondtech::UnloadBackToPTFE; },
    //        5000));

    //    // disengaging idler
    //    REQUIRE(ff.State() == FeedToBondtech::DisengagingIdler);
    //    REQUIRE(WhileCondition(
    //        ff,
    //        [&]() { return mi::idler.Engaged(); },
    //        5000));

    //    CHECK(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5)); // @@TODO constants
    //    CHECK(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(0));

    //    // state machine finished ok, the green LED should be on
    //    REQUIRE(ff.State() == FeedToBondtech::OK);
    //    REQUIRE(ml::leds.LedOn(mg::globals.ActiveSlot(), ml::Color::green));

    //    REQUIRE(ff.Step() == true); // the automaton finished its work, any consecutive calls to Step must return true
    //}

    //TEST_CASE("feed_to_finda::FINDA_failed", "[feed_to_finda]") {
    //    using namespace logic;

    //    ForceReinitAllAutomata();

    //    FeedToBondtech ff;
    //    main_loop();

    //    // restart the automaton - we want the limited version of the feed
    //    ff.Reset(true);

    //    REQUIRE(ff.State() == FeedToBondtech::EngagingIdler);

    //    // it should have instructed the selector and idler to move to slot 1
    //    // check if the idler and selector have the right command
    //    CHECK(mm::axes[mm::Idler].targetPos == mi::Idler::SlotPosition(0));
    //    CHECK(mm::axes[mm::Selector].targetPos == ms::Selector::SlotPosition(0));

    //    // engaging idler
    //    REQUIRE(WhileCondition(
    //        ff,
    //        [&]() { return !mi::idler.Engaged(); },
    //        5000));

    //    CHECK(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(0));
    //    CHECK(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(0));

    //    // idler engaged, we'll start pushing filament
    //    REQUIRE(ff.State() == FeedToBondtech::PushingFilament);
    //    // at least at the beginning the LED should shine green (it should be blinking, but this mode has been already verified in the LED's unit test)
    //    REQUIRE(ml::leds.Mode(mg::globals.ActiveSlot(), ml::Color::green) == ml::blink0);

    //    // now let the filament be pushed into the FINDA - but we make sure the FINDA doesn't trigger at all
    //    ha::ReinitADC(1, ha::TADCData({ 0 }), 100);

    //    REQUIRE(WhileCondition(
    //        ff, // boo, this formatting is UGLY!
    //        [&]() { return ff.State() == FeedToBondtech::PushingFilament; },
    //        5000));

    //    // the FINDA didn't trigger, we should be in the Failed state
    //    REQUIRE(ff.State() == FeedToBondtech::Failed);
    //    REQUIRE(ml::leds.Mode(mg::globals.ActiveSlot(), ml::Color::green) == ml::off);
    //    REQUIRE(ml::leds.Mode(mg::globals.ActiveSlot(), ml::Color::red) == ml::blink0);

    //    REQUIRE(ff.Step() == true); // the automaton finished its work, any consecutive calls to Step must return true
}
