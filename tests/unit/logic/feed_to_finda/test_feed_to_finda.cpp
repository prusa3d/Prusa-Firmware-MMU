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

#include "../../../../src/logic/feed_to_finda.h"

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

TEST_CASE("feed_to_finda::feed_phase_unlimited", "[feed_to_finda]") {
    using namespace logic;

    ForceReinitAllAutomata();

    FeedToFinda ff;
    main_loop();

    // restart the automaton
    ff.Reset(false);

    REQUIRE(ff.State() == FeedToFinda::EngagingIdler);

    // it should have instructed the selector and idler to move to slot 1
    // check if the idler and selector have the right command
    CHECK(mm::axes[mm::Idler].targetPos == mi::Idler::SlotPosition(0).v);
    CHECK(mm::axes[mm::Selector].targetPos == ms::Selector::SlotPosition(0));
    CHECK(mm::axes[mm::Idler].enabled == true);

    // engaging idler
    REQUIRE(WhileCondition(
        ff,
        [&](int) { return !mi::idler.Engaged(); },
        5000));

    CHECK(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(0).v);
    CHECK(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(0));

    // idler engaged, selector in position, we'll start pushing filament
    REQUIRE(ff.State() == FeedToFinda::PushingFilament);
    // at least at the beginning the LED should shine green (it should be blinking, but this mode has been already verified in the LED's unit test)
    REQUIRE(ml::leds.LedOn(mg::globals.ActiveSlot(), ml::Color::green));

    // now let the filament be pushed into the FINDA - do 500 steps without triggering the condition
    // and then let the simulated ADC channel 1 create a FINDA switch
    ha::ReinitADC(config::findaADCIndex, ha::TADCData({ 600, 700, 800, 900 }), 1);

    REQUIRE(WhileCondition(
        ff,
        [&](int) { return ff.State() == FeedToFinda::PushingFilament; },
        1500));
    // From now on the FINDA is reported as ON

    // unloading back to PTFE
    REQUIRE(ff.State() == FeedToFinda::UnloadBackToPTFE);
    REQUIRE(WhileCondition(
        ff,
        [&](int) { return ff.State() == FeedToFinda::UnloadBackToPTFE; },
        5000));

    //    // disengaging idler
    //    REQUIRE(ff.State() == FeedToFinda::DisengagingIdler);
    //    REQUIRE(WhileCondition(
    //        ff,
    //        [&](int) { return mi::idler.Engaged(); },
    //        5000));

    CHECK(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(0).v); // @@TODO constants
    CHECK(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(0));

    // state machine finished ok, the green LED should be on
    REQUIRE(ff.State() == FeedToFinda::OK);
    REQUIRE(ml::leds.Mode(mg::globals.ActiveSlot(), ml::green) == ml::blink0);

    REQUIRE(ff.Step() == true); // the automaton finished its work, any consecutive calls to Step must return true
}

TEST_CASE("feed_to_finda::FINDA_failed", "[feed_to_finda]") {
    using namespace logic;

    ForceReinitAllAutomata();

    FeedToFinda ff;
    main_loop();

    // restart the automaton - we want the limited version of the feed
    ff.Reset(true);

    REQUIRE(ff.State() == FeedToFinda::EngagingIdler);

    // it should have instructed the selector and idler to move to slot 1
    // check if the idler and selector have the right command
    CHECK(mm::axes[mm::Idler].targetPos == mi::Idler::SlotPosition(0).v);
    CHECK(mm::axes[mm::Selector].targetPos == ms::Selector::SlotPosition(0));

    // engaging idler
    REQUIRE(WhileCondition(
        ff,
        [&](int) { return !mi::idler.Engaged(); },
        5000));

    CHECK(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(0).v);
    CHECK(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(0));

    // idler engaged, we'll start pushing filament
    REQUIRE(ff.State() == FeedToFinda::PushingFilament);
    // at least at the beginning the LED should shine green (it should be blinking, but this mode has been already verified in the LED's unit test)
    REQUIRE(ml::leds.Mode(mg::globals.ActiveSlot(), ml::Color::green) == ml::blink0);

    // now let the filament be pushed into the FINDA - but we make sure the FINDA doesn't trigger at all
    ha::ReinitADC(config::findaADCIndex, ha::TADCData({ 0 }), 100);

    REQUIRE(WhileCondition(
        ff,
        [&](int) { return ff.State() == FeedToFinda::PushingFilament; },
        5000));

    // the FINDA didn't trigger, we should be in the Failed state
    REQUIRE(ff.State() == FeedToFinda::Failed);
    REQUIRE(ml::leds.Mode(mg::globals.ActiveSlot(), ml::Color::green) == ml::off);
    REQUIRE(ml::leds.Mode(mg::globals.ActiveSlot(), ml::Color::red) == ml::blink0);

    REQUIRE(ff.Step() == true); // the automaton finished its work, any consecutive calls to Step must return true
}
