#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators_range.hpp"

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
#include "../../../../src/modules/timebase.h"

#include "../../../../src/logic/unload_to_finda.h"

#include "../../modules/stubs/stub_adc.h"
#include "../../modules/stubs/stub_timebase.h"

#include "../stubs/main_loop_stub.h"
#include "../stubs/stub_motion.h"

using namespace std::placeholders;

namespace ha = hal::adc;

void UnloadToFindaCommonSetup(logic::UnloadToFinda &ff, uint8_t retryAttempts) {
    ForceReinitAllAutomata();
    REQUIRE(EnsureActiveSlotIndex(0, mg::FilamentLoadState::AtPulley));

    // we need finda ON
    SetFINDAStateAndDebounce(true);
    // fsensor should be ON
    SetFSensorStateAndDebounce(true);
    // and MMU "thinks" it has the filament loaded
    mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::FilamentLoadState::InNozzle);

    // restart the automaton - just 1 attempt
    ff.Reset(retryAttempts);

    REQUIRE(ff.State() == logic::UnloadToFinda::EngagingIdler);

    // it should have instructed the selector and idler to move to slot 1
    // check if the idler and selector have the right command
    CHECK(mm::AxisNearestTargetPos(mm::Idler) == mi::Idler::IntermediateSlotPosition(0).v);
    CHECK(mm::AxisNearestTargetPos(mm::Selector) == ms::Selector::SlotPosition(0).v);

    REQUIRE(SimulateEngageIdlerPartially(ff));
}

void UnloadToFindaCommonTurnOffFSensor(logic::UnloadToFinda &ff) {
    // turn off fsensor - the printer freed the filament from the gears
    SetFSensorStateAndDebounce(false);

    // make sure we step ff to handle turned-off fsensor
    ff.Step();

    REQUIRE(ff.State() == logic::UnloadToFinda::UnloadingToFinda);
    CHECK(mm::axes[mm::Pulley].enabled == true);
    CHECK(mm::AxisNearestTargetPos(mm::Idler) == mi::Idler::SlotPosition(0).v);

    REQUIRE(SimulateEngageIdlerFully(ff));

    // now pulling the filament until finda triggers
    REQUIRE(ff.State() == logic::UnloadToFinda::WaitingForFINDA);
}

TEST_CASE("unload_to_finda::regular_unload", "[unload_to_finda]") {
    logic::UnloadToFinda ff;
    UnloadToFindaCommonSetup(ff, 1);
    UnloadToFindaCommonTurnOffFSensor(ff);

    REQUIRE(WhileCondition(ff, std::bind(SimulateUnloadToFINDA, _1, 10, 1000), 1100));

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
    logic::UnloadToFinda ff;
    UnloadToFindaCommonSetup(ff, 1);
    UnloadToFindaCommonTurnOffFSensor(ff);

    // no changes to FINDA during unload - we'll pretend it never triggers
    // but set FSensor correctly
    uint32_t unlSteps = 10 + mm::unitToSteps<mm::P_pos_t>(config::maximumBowdenLength + config::feedToFinda + config::filamentMinLoadedToMMU);
    REQUIRE_FALSE(WhileCondition(ff, std::bind(SimulateUnloadToFINDA, _1, 10, 150000), unlSteps));

    REQUIRE(ff.State() == logic::UnloadToFinda::FailedFINDA);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InSelector);
}

TEST_CASE("unload_to_finda::unload_without_FSensor_trigger", "[unload_to_finda]") {
    logic::UnloadToFinda ff;
    UnloadToFindaCommonSetup(ff, 1);

    // no changes to FSensor during unload - we'll pretend it never triggers
    // time-out in 4 seconds
    mt::IncMillis(4000);

    main_loop();
    ff.Step();

    // no pulling actually starts, because the fsensor didn't turn off and the time-out elapsed
    REQUIRE(ff.State() == logic::UnloadToFinda::FailedFSensor);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InNozzle);
}

TEST_CASE("unload_to_finda::unload_repeated", "[unload_to_finda]") {
    logic::UnloadToFinda ff;
    UnloadToFindaCommonSetup(ff, 2);

    UnloadToFindaCommonTurnOffFSensor(ff);

    // remember raw Pulley pos for tweaking the steps below
    // because a 20mm (config::fsensorToNozzleAvoidGrindUnload)
    // move is being executed while the Idler is fully engaging
    // It is roughly -90 steps
    // int32_t pulleySteppedAlready = mm::axes[config::Pulley].pos;

    // no changes to FINDA during unload - we'll pretend it never triggers
    // but set FSensor correctly
    // In this case it is vital to correctly compute the amount of steps
    // to make the unload state machine restart after the 1st attempt
    // The number of steps must be more than what the state machine expects for FINDA to trigger.
    uint32_t unlSteps = 1 + mm::unitToSteps<mm::P_pos_t>(
                            // standard fast move distance
                            config::maximumBowdenLength + config::feedToFinda + config::filamentMinLoadedToMMU
                            // slow start move distance
                            + config::fsensorToNozzleAvoidGrindUnload);
    // compensation
    // + pulleySteppedAlready;
    REQUIRE_FALSE(WhileCondition(ff, std::bind(SimulateUnloadToFINDA, _1, 10, 150000), unlSteps));

    main_loop();
    ff.Step();

    REQUIRE_FALSE(mi::idler.HomingValid());
    REQUIRE(ff.State() == logic::UnloadToFinda::EngagingIdler);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InSelector);

    HomeIdler();

    main_loop();
    ff.Step();
    REQUIRE(ff.State() == logic::UnloadToFinda::UnloadingToFinda);

    SimulateEngageIdlerPartially(ff);

    main_loop();
    ff.Step();

    SimulateEngageIdlerFully(ff);

    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InSelector);

    // make arbitrary amount of steps
    uint32_t steps = GENERATE(range(1, 50));
    for (uint32_t i = 0; i < steps; ++i) {
        main_loop();
        ff.Step();
    }

    REQUIRE(ff.State() == logic::UnloadToFinda::WaitingForFINDA);
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
