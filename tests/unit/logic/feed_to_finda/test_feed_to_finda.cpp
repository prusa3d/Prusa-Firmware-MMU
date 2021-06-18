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

template <typename COND>
bool WhileCondition(logic::FeedToFinda &ff, COND cond, uint32_t maxLoops = 5000) {
    while (cond() && --maxLoops) {
        main_loop();
        ff.Step();
    }
    return maxLoops > 0;
}

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
    CHECK(modules::motion::axes[modules::motion::Idler].targetPos == 0); // @@TODO constants
    CHECK(modules::motion::axes[modules::motion::Selector].targetPos == 0); // @@TODO constants
    CHECK(modules::motion::axes[modules::motion::Idler].enabled == true); // @@TODO constants
    CHECK(modules::motion::axes[modules::motion::Selector].enabled == true); // @@TODO constants

    // engaging idler
    REQUIRE(WhileCondition(
        ff,
        [&]() { return !modules::idler::idler.Engaged(); },
        5000));

    // idler engaged, selector in position, we'll start pushing filament
    REQUIRE(ff.State() == FeedToFinda::PushingFilament);
    // at least at the beginning the LED should shine green (it should be blinking, but this mode has been already verified in the LED's unit test)
    REQUIRE(modules::leds::leds.LedOn(modules::globals::globals.ActiveSlot(), modules::leds::Color::green));

    // now let the filament be pushed into the FINDA - do 500 steps without triggering the condition
    // and then let the simulated ADC channel 1 create a FINDA switch
    hal::adc::ReinitADC(1, hal::adc::TADCData({ 600, 700, 800, 900 }), 1);

    REQUIRE(WhileCondition(
        ff,
        [&]() { return ff.State() == FeedToFinda::PushingFilament; },
        1500));
    // From now on the FINDA is reported as ON

    // unloading back to PTFE
    REQUIRE(ff.State() == FeedToFinda::UnloadBackToPTFE);
    REQUIRE(WhileCondition(
        ff,
        [&]() { return ff.State() == FeedToFinda::UnloadBackToPTFE; },
        5000));

    // disengaging idler
    REQUIRE(ff.State() == FeedToFinda::DisengagingIdler);
    REQUIRE(WhileCondition(
        ff,
        [&]() { return modules::idler::idler.Engaged(); },
        5000));

    // state machine finished ok, the green LED should be on
    REQUIRE(ff.State() == FeedToFinda::OK);
    REQUIRE(modules::leds::leds.LedOn(modules::globals::globals.ActiveSlot(), modules::leds::Color::green));

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
    CHECK(modules::motion::axes[modules::motion::Idler].targetPos == 0); // @@TODO constants
    CHECK(modules::motion::axes[modules::motion::Selector].targetPos == 0); // @@TODO constants

    // engaging idler
    REQUIRE(WhileCondition(
        ff,
        [&]() { return !modules::idler::idler.Engaged(); },
        5000));

    // idler engaged, we'll start pushing filament
    REQUIRE(ff.State() == FeedToFinda::PushingFilament);
    // at least at the beginning the LED should shine green (it should be blinking, but this mode has been already verified in the LED's unit test)
    REQUIRE(modules::leds::leds.LedOn(modules::globals::globals.ActiveSlot(), modules::leds::Color::green));

    // now let the filament be pushed into the FINDA - but we make sure the FINDA doesn't trigger at all
    hal::adc::ReinitADC(1, hal::adc::TADCData({ 0 }), 100);

    REQUIRE(WhileCondition(
        ff, // boo, this formatting is UGLY!
        [&]() { return ff.State() == FeedToFinda::PushingFilament; },
        5000));

    // the FINDA didn't trigger, we should be in the Failed state
    REQUIRE(ff.State() == FeedToFinda::Failed);

    REQUIRE(ff.Step() == true); // the automaton finished its work, any consecutive calls to Step must return true
}
