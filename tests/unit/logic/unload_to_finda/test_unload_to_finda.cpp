#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators_range.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"

#include <functional>

#include "../../../../src/modules/buttons.h"
#include "../../../../src/modules/finda.h"
#include "../../../../src/modules/fsensor.h"
#include "../../../../src/modules/globals.h"
#include "../../../../src/modules/idler.h"
#include "../../../../src/modules/leds.h"
#include "../../../../src/modules/motion.h"
#include "../../../../src/modules/permanent_storage.h"
#include "../../../../src/modules/selector.h"

#include "../../../../src/logic/unload_to_finda.h"

#include "../../modules/stubs/stub_adc.h"

#include "../stubs/main_loop_stub.h"
#include "../stubs/stub_motion.h"

using Catch::Matchers::Equals;
using namespace std::placeholders;

namespace ha = hal::adc;

TEST_CASE("unload_to_finda::regular_unload", "[unload_to_finda]") {
    ForceReinitAllAutomata();
    REQUIRE(EnsureActiveSlotIndex(0, mg::FilamentLoadState::AtPulley));

    // we need finda ON
    SetFINDAStateAndDebounce(true);
    // fsensor should be ON
    SetFSensorStateAndDebounce(true);
    // and MMU "thinks" it has the filament loaded
    mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::FilamentLoadState::InNozzle);

    logic::UnloadToFinda ff;

    // restart the automaton - just 1 attempt
    ff.Reset(1);

    REQUIRE(ff.State() == logic::UnloadToFinda::EngagingIdler);

    // it should have instructed the selector and idler to move to slot 1
    // check if the idler and selector have the right command
    CHECK(mm::AxisNearestTargetPos(mm::Idler) == mi::Idler::SlotPosition(0).v);
    CHECK(mm::AxisNearestTargetPos(mm::Selector) == ms::Selector::SlotPosition(0).v);

    // engaging idler
    REQUIRE(WhileCondition(
        ff,
        [&](uint32_t) { return !mi::idler.Engaged(); },
        5000));
    CHECK(mm::axes[mm::Pulley].enabled == true);

    // now pulling the filament until finda triggers
    REQUIRE(ff.State() == logic::UnloadToFinda::UnloadingFromFSensor);
    REQUIRE(WhileCondition(ff, std::bind(SimulateUnloadToFINDA, _1,
                                   mm::unitToSteps<mm::P_pos_t>(mg::globals.FSensorUnloadCheck_mm()) / 4, // make fsensor trigger roughly in the first 1/4 of the check distance
                                   mm::unitToSteps<mm::P_pos_t>(config::minimumBowdenLength)),
        200'000));

    REQUIRE(ff.State() == logic::UnloadToFinda::OK);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InSelector);
}

TEST_CASE("unload_to_finda::no_sense_FINDA_upon_start", "[unload_to_finda]") {
    ForceReinitAllAutomata(); // that implies FINDA OFF which should really not happen for an unload call
    REQUIRE(EnsureActiveSlotIndex(0, mg::FilamentLoadState::AtPulley));

    logic::UnloadToFinda ff;

    // restart the automaton - just 1 attempt
    ff.Reset(1);

    // the state machine should accept the unpressed FINDA as no-fillament-loaded
    // thus should immediately end in the OK state
    REQUIRE(ff.State() == logic::UnloadToFinda::OK);
}

TEST_CASE("unload_to_finda::unload_without_FINDA_trigger", "[unload_to_finda]") {
    ForceReinitAllAutomata();
    REQUIRE(EnsureActiveSlotIndex(0, mg::FilamentLoadState::AtPulley));

    // we need finda ON
    SetFINDAStateAndDebounce(true);
    // fsensor should be ON
    SetFSensorStateAndDebounce(true);
    // and MMU "thinks" it has the filament loaded
    mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::FilamentLoadState::InNozzle);

    logic::UnloadToFinda ff;

    // restart the automaton - just 1 attempt
    ff.Reset(1);

    REQUIRE(ff.State() == logic::UnloadToFinda::EngagingIdler);

    // it should have instructed the selector and idler to move to slot 1
    // check if the idler and selector have the right command
    CHECK(mm::AxisNearestTargetPos(mm::Idler) == mi::Idler::SlotPosition(0).v);
    CHECK(mm::AxisNearestTargetPos(mm::Selector) == ms::Selector::SlotPosition(0).v);
    CHECK(mm::axes[mm::Idler].enabled == true);

    // engaging idler
    REQUIRE(WhileCondition(
        ff,
        [&](uint32_t) { return !mi::idler.Engaged(); },
        5000));

    // now pulling the filament until finda triggers
    REQUIRE(ff.State() == logic::UnloadToFinda::UnloadingFromFSensor);

    // no changes to FINDA during unload - we'll pretend it never triggers
    // but set FSensor correctly
    REQUIRE_FALSE(WhileCondition(ff, std::bind(SimulateUnloadToFINDA, _1,
                                         mm::unitToSteps<mm::P_pos_t>(mg::globals.FSensorUnloadCheck_mm()) / 4, // make fsensor trigger roughly in the first 1/4 of the check distance
                                         mm::unitToSteps<mm::P_pos_t>(config::maximumBowdenLength * 2) // set finda trigger beyond the total allowed number of steps (i.e. not trigger)
                                         ),
        mm::unitToSteps<mm::P_pos_t>(config::maximumBowdenLength) // limit the number of loops to max bowden length steps
        ));

    REQUIRE(ff.State() == logic::UnloadToFinda::FailedFINDA);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InSelector);
}

TEST_CASE("unload_to_finda::unload_without_FSensor_trigger", "[unload_to_finda]") {
    ForceReinitAllAutomata();
    REQUIRE(EnsureActiveSlotIndex(0, mg::FilamentLoadState::AtPulley));

    // we need finda ON
    SetFINDAStateAndDebounce(true);
    // fsensor should be ON
    SetFSensorStateAndDebounce(true);
    // and MMU "thinks" it has the filament loaded
    mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::FilamentLoadState::InNozzle);

    logic::UnloadToFinda ff;

    // restart the automaton - just 1 attempt
    ff.Reset(1);

    REQUIRE(ff.State() == logic::UnloadToFinda::EngagingIdler);

    // it should have instructed the selector and idler to move to slot 1
    // check if the idler and selector have the right command
    CHECK(mm::AxisNearestTargetPos(mm::Idler) == mi::Idler::SlotPosition(0).v);
    CHECK(mm::AxisNearestTargetPos(mm::Selector) == ms::Selector::SlotPosition(0).v);
    CHECK(mm::axes[mm::Idler].enabled == true);

    // engaging idler
    REQUIRE(WhileCondition(
        ff,
        [&](uint32_t) { return !mi::idler.Engaged(); },
        5000));

    // now pulling the filament until finda triggers
    REQUIRE(ff.State() == logic::UnloadToFinda::UnloadingFromFSensor);

    // no changes to FSensor during unload - we'll pretend it never triggers
    // but set FINDA correctly
    REQUIRE(WhileCondition(ff, std::bind(SimulateUnloadToFINDA, _1,
                                   mm::unitToSteps<mm::P_pos_t>(config::maximumBowdenLength * 2), // no changes to FSensor during unload - we'll pretend it never triggers
                                   mm::unitToSteps<mm::P_pos_t>(config::minimumBowdenLength) // trigger finda roughly at the expected distance (should never reach though)
                                   ),
        mm::unitToSteps<mm::P_pos_t>(config::maximumBowdenLength) // limit the number of loops to max bowden length steps
        ));

    REQUIRE(ff.State() == logic::UnloadToFinda::FailedFSensor);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InSelector);
}

TEST_CASE("unload_to_finda::unload_repeated", "[unload_to_finda]") {
    ForceReinitAllAutomata();
    REQUIRE(EnsureActiveSlotIndex(0, mg::FilamentLoadState::AtPulley));

    // we need finda ON
    SetFINDAStateAndDebounce(true);
    // fsensor should be ON
    SetFSensorStateAndDebounce(true);
    // and MMU "thinks" it has the filament loaded
    mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::FilamentLoadState::InNozzle);

    logic::UnloadToFinda ff;

    // restart the automaton - 2 attempts
    ff.Reset(2);

    REQUIRE(ff.State() == logic::UnloadToFinda::EngagingIdler);

    // it should have instructed the selector and idler to move to slot 1
    // check if the idler and selector have the right command
    CHECK(mm::AxisNearestTargetPos(mm::Idler) == mi::Idler::SlotPosition(0).v);
    CHECK(mm::AxisNearestTargetPos(mm::Selector) == ms::Selector::SlotPosition(0).v);
    CHECK(mm::axes[mm::Idler].enabled == true);

    // engaging idler
    REQUIRE(WhileCondition(
        ff,
        [&](uint32_t) { return !mi::idler.Engaged(); },
        5000));

    // now pulling the filament until finda triggers
    REQUIRE(ff.State() == logic::UnloadToFinda::UnloadingFromFSensor);

    // no changes to FINDA during unload - we'll pretend it never triggers
    // but set FSensor correctly
    // In this case it is vital to correctly compute the amount of steps
    // to make the unload state machine restart after the 1st attempt
    uint32_t unlSteps = 3 + mm::unitToSteps<mm::P_pos_t>(config::defaultBowdenLength + config::feedToFinda + config::filamentMinLoadedToMMU - mg::globals.FSensorUnloadCheck_mm());
    REQUIRE_FALSE(WhileCondition(
        ff,
        std::bind(SimulateUnloadToFINDA, _1,
            mm::unitToSteps<mm::P_pos_t>(mg::globals.FSensorUnloadCheck_mm()) / 4, // make fsensor trigger roughly in the first 1/4 of the check distance
            mm::unitToSteps<mm::P_pos_t>(config::maximumBowdenLength * 2) // do not allow finda to trigger within the specified range
            ),
        unlSteps));

    main_loop();
    ff.Step();

    REQUIRE(ff.State() == logic::UnloadToFinda::EngagingIdler);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InSelector);

    main_loop();
    ff.Step();

    REQUIRE(ff.State() == logic::UnloadToFinda::UnloadingFromFSensor);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InSelector);

    // make arbitrary amount of steps
    uint32_t steps = GENERATE(range(1, 50));
    for (uint32_t i = 0; i < steps; ++i) {
        main_loop();
        ff.Step();
    }

    // technically we are in the WaitingForFINDA state, but we want to tell the printer to rotate the E-motor after each unload retry
    // -> therefore we check for the UnloadingFromFSensor state
    REQUIRE(ff.State() == logic::UnloadToFinda::UnloadingFromFSensor);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InSelector);

    // now turn FINDA off - shall respond immediately
    hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::low);

    main_loop();
    ff.Step();

    REQUIRE(ff.State() == logic::UnloadToFinda::OK);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InSelector);

    main_loop();
    REQUIRE(ff.Step() == true);
}
