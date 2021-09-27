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

namespace ha = hal::adc;

TEST_CASE("feed_to_bondtech::feed_phase_unlimited", "[feed_to_bondtech]") {
    using namespace logic;

    ForceReinitAllAutomata();

    FeedToBondtech fb;
    main_loop();

    // restart the automaton
    fb.Reset(1);

    REQUIRE(fb.State() == FeedToBondtech::EngagingIdler);

    // it should have instructed the selector and idler to move to slot 0
    // check if the idler and selector have the right command
    CHECK(mm::axes[mm::Idler].targetPos == mi::Idler::SlotPosition(0).v);
    CHECK(mm::axes[mm::Selector].targetPos == ms::Selector::SlotPosition(0).v);
    CHECK(mm::axes[mm::Idler].enabled == true);

    // engaging idler
    REQUIRE(WhileCondition(
        fb,
        [&](int) { return !mi::idler.Engaged(); },
        5000));

    CHECK(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(0).v);
    CHECK(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(0).v);

    // idler engaged, selector in position, we'll start pushing filament
    REQUIRE(fb.State() == FeedToBondtech::PushingFilament);
    // at least at the beginning the LED should shine green (it should be blinking, but this mode has been already verified in the LED's unit test)
    REQUIRE(ml::leds.LedOn(mg::globals.ActiveSlot(), ml::green));

    REQUIRE(WhileCondition(
        fb,
        [&](int step) {
        if( step == 1000 ){
            mfs::fsensor.ProcessMessage(true);
        }
        return fb.State() == FeedToBondtech::PushingFilament; },
        1500));

    REQUIRE(mfs::fsensor.Pressed());

    // disengaging idler
    REQUIRE(fb.State() == FeedToBondtech::DisengagingIdler);
    REQUIRE(WhileCondition(
        fb,
        [&](int) { return fb.State() == FeedToBondtech::DisengagingIdler; },
        5000));

    CHECK(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5).v);
    CHECK(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(0).v);

    // state machine finished ok, the green LED should be on
    REQUIRE(fb.State() == FeedToBondtech::OK);
    REQUIRE(ml::leds.LedOn(mg::globals.ActiveSlot(), ml::green));

    REQUIRE(fb.Step() == true); // the automaton finished its work, any consecutive calls to Step must return true
}
