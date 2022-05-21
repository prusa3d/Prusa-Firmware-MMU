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

#include "../stubs/homing.h"
#include "../stubs/main_loop_stub.h"
#include "../stubs/stub_motion.h"

#include <functional>

using Catch::Matchers::Equals;
using namespace std::placeholders;

#include "../helpers/helpers.ipp"

// needs to be a separate function otherwise gdb has issues setting breakpoints inside
bool FeedingToFindaStep(logic::CommandBase &tc, uint32_t step, uint32_t triggerAt) {
    if (step == triggerAt) { // on specified stepNr make FINDA trigger
        hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::high);
    } else if (step >= triggerAt + config::findaDebounceMs + 2) {
        REQUIRE(mf::finda.Pressed() == true);
    }
    return tc.TopLevelState() == ProgressCode::FeedingToFinda;
}

void FeedingToFinda(logic::ToolChange &tc, uint8_t toSlot, uint32_t triggerAt = 1000) {
    // feeding to finda
    REQUIRE(WhileCondition(tc, std::bind(FeedingToFindaStep, std::ref(tc), _1, triggerAt), 200'000UL));
    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InSelector, toSlot, toSlot, true, true, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::FeedingToBondtech));
}

void FeedingToBondtech(logic::ToolChange &tc, uint8_t toSlot) {
    // james is feeding fast and then slowly
    // FSensor must not trigger too early
    REQUIRE_FALSE(mfs::fsensor.Pressed());
    REQUIRE(WhileCondition(
        tc,
        [&](uint32_t step) -> bool {
        if(step == mm::unitToSteps<mm::P_pos_t>(config::minimumBowdenLength)+10){ // on the correct step make filament sensor trigger
            mfs::fsensor.ProcessMessage(true);
        }
        return tc.TopLevelState() == ProgressCode::FeedingToBondtech; },
        mm::unitToSteps<mm::P_pos_t>(config::minimumBowdenLength) + 10000));
    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InNozzle, mi::Idler::IdleSlotIndex(), toSlot, true, false, ml::on, ml::off, ErrorCode::OK, ProgressCode::OK));
}

void CheckFinishedCorrectly(logic::ToolChange &tc, uint8_t toSlot) {
    REQUIRE(tc.TopLevelState() == ProgressCode::OK);
    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::InNozzle);
    REQUIRE(mg::globals.ActiveSlot() == toSlot);
}

void ToolChange(logic::ToolChange &tc, uint8_t fromSlot, uint8_t toSlot) {
    ForceReinitAllAutomata();

    SetFINDAStateAndDebounce(true);
    mfs::fsensor.ProcessMessage(true);
    EnsureActiveSlotIndex(fromSlot, mg::FilamentLoadState::InNozzle);

    // restart the automaton
    tc.Reset(toSlot);

    REQUIRE(WhileCondition(
        tc,
        [&](uint32_t step) -> bool {
        if(step == 20){ // on 20th step make FSensor switch off
            mfs::fsensor.ProcessMessage(false);
        } else if(step == mm::unitToSteps<mm::P_pos_t>(config::minimumBowdenLength)){ // on 2000th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::low);
        }
        return tc.TopLevelState() == ProgressCode::UnloadingFilament; },
        200000UL));

    //    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::AtPulley);
    REQUIRE(VerifyState2(tc, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), fromSlot, false, false, toSlot, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::FeedingToFinda));

    FeedingToFinda(tc, toSlot);

    FeedingToBondtech(tc, toSlot);

    CheckFinishedCorrectly(tc, toSlot);
}

void NoToolChange(logic::ToolChange &tc, uint8_t fromSlot, uint8_t toSlot) {
    ForceReinitAllAutomata();

    // the filament is LOADED
    SetFINDAStateAndDebounce(true);
    mfs::fsensor.ProcessMessage(true);
    EnsureActiveSlotIndex(fromSlot, mg::FilamentLoadState::InNozzle);

    REQUIRE(VerifyEnvironmentState(mg::FilamentLoadState::InNozzle, mi::Idler::IdleSlotIndex(), toSlot, true, false, ml::off, ml::off));

    // restart the automaton
    tc.Reset(toSlot);

    // should not do anything
    REQUIRE(tc.TopLevelState() == ProgressCode::OK);
    REQUIRE(tc.Error() == ErrorCode::OK);
}

void JustLoadFilament(logic::ToolChange &tc, uint8_t slot) {
    ForceReinitAllAutomata();

    EnsureActiveSlotIndex(slot, mg::FilamentLoadState::AtPulley);

    // verify filament NOT loaded
    REQUIRE(VerifyEnvironmentState(mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), slot, false, false, ml::off, ml::off));

    // restart the automaton
    tc.Reset(slot);

    FeedingToFinda(tc, slot);

    FeedingToBondtech(tc, slot);

    CheckFinishedCorrectly(tc, slot);
}

TEST_CASE("tool_change::test0", "[tool_change]") {
    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount; ++toSlot) {
            logic::ToolChange tc;
            if (fromSlot != toSlot) {
                ToolChange(tc, fromSlot, toSlot);
            } else {
                NoToolChange(tc, fromSlot, toSlot);
            }
        }
    }
}

TEST_CASE("tool_change::invalid_slot", "[tool_change]") {
    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        logic::ToolChange tc;
        InvalidSlot<logic::ToolChange>(tc, fromSlot, config::toolCount);
    }
}

TEST_CASE("tool_change::state_machine_reusal", "[tool_change]") {
    logic::ToolChange tc;

    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount + 2; ++toSlot) {
            if (toSlot >= config::toolCount) {
                InvalidSlot<logic::ToolChange>(tc, fromSlot, toSlot);
            } else if (fromSlot != toSlot) {
                ToolChange(tc, fromSlot, toSlot);
            } else {
                NoToolChange(tc, fromSlot, toSlot);
            }
        }
    }
}

TEST_CASE("tool_change::same_slot_just_unloaded_filament", "[tool_change]") {
    for (uint8_t toSlot = 0; toSlot < config::toolCount; ++toSlot) {
        logic::ToolChange tc;
        JustLoadFilament(tc, toSlot);
    }
}

void ToolChangeFailLoadToFinda(logic::ToolChange &tc, uint8_t fromSlot, uint8_t toSlot) {
    ForceReinitAllAutomata();

    SetFINDAStateAndDebounce(true);
    mfs::fsensor.ProcessMessage(true);
    EnsureActiveSlotIndex(fromSlot, mg::FilamentLoadState::InNozzle);

    // restart the automaton
    tc.Reset(toSlot);

    REQUIRE(WhileCondition(tc, std::bind(SimulateUnloadToFINDA, _1, 100, 2'000), 200'000));
    REQUIRE(WhileTopState(tc, ProgressCode::UnloadingFilament, 5000));

    REQUIRE(mg::globals.FilamentLoaded() == mg::FilamentLoadState::AtPulley);

    // feeding to finda, but fails - do not trigger FINDA
    REQUIRE(WhileTopState(tc, ProgressCode::FeedingToFinda, 50000UL));

    // should end up in error disengage idler
    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InSelector, toSlot, toSlot, false, true, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_ON, ProgressCode::ERRDisengagingIdler));
    REQUIRE(WhileTopState(tc, ProgressCode::ERRDisengagingIdler, 5000));
}

void ToolChangeFailLoadToFindaLeftBtn(logic::ToolChange &tc, uint8_t toSlot) {
    // now waiting for user input
    PressButtonAndDebounce(tc, mb::Left);

    REQUIRE(WhileTopState(tc, ProgressCode::ERREngagingIdler, 5000UL));
    ClearButtons(tc);

    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InSelector, toSlot, toSlot, false, true, ml::off, ml::blink0, ErrorCode::RUNNING, ProgressCode::ERRHelpingFilament));

    // try push more, if FINDA triggers, continue loading
    REQUIRE(WhileCondition(
        tc,
        [&](uint32_t step) -> bool {
        if(step == 20){ // on 20th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::high);
        }
        return tc.TopLevelState() == ProgressCode::ERRHelpingFilament; },
        2000UL));

    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InSelector, toSlot, toSlot, true, true, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::FeedingToBondtech));

    FeedingToBondtech(tc, toSlot);

    CheckFinishedCorrectly(tc, toSlot);
}

void ToolChangeFailLoadToFindaMiddleBtn(logic::ToolChange &tc, uint8_t toSlot) {
    // now waiting for user input
    PressButtonAndDebounce(tc, mb::Middle);

    REQUIRE(WhileCondition(
        tc,
        [&](uint32_t step) -> bool {
        if(step == 2000){ // on 2000th step make FINDA trigger
            hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::low);
        }
        return tc.TopLevelState() == ProgressCode::UnloadingFilament; },
        200000UL));
    REQUIRE(VerifyState(tc, mg::FilamentLoadState::AtPulley, mi::Idler::IdleSlotIndex(), toSlot, false, false, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::FeedingToFinda));

    ClearButtons(tc);

    // retry the whole operation
    // beware - the FeedToFinda state machine will leverage the already engaged Idler,
    // so the necessary number of steps to reach the FINDA is quite low (~200 was lowest once tested)
    // without running short of max distance of Pulley to travel
    FeedingToFinda(tc, toSlot, 200);

    FeedingToBondtech(tc, toSlot);

    CheckFinishedCorrectly(tc, toSlot);
}

void ToolChangeFailLoadToFindaRightBtnFINDA_FSensor(logic::ToolChange &tc, uint8_t toSlot) {
    // now waiting for user input - press FINDA and FSensor
    hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::high);
    mfs::fsensor.ProcessMessage(true);
    PressButtonAndDebounce(tc, mb::Right);

    CheckFinishedCorrectly(tc, toSlot);

    ClearButtons(tc);
}

void ToolChangeFailLoadToFindaRightBtnFINDA(logic::ToolChange &tc, uint8_t toSlot) {
    // now waiting for user input - press FINDA
    hal::gpio::WritePin(FINDA_PIN, hal::gpio::Level::high);
    PressButtonAndDebounce(tc, mb::Right);

    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InSelector, mi::Idler::IdleSlotIndex(), toSlot, true, false, ml::off, ml::blink0, ErrorCode::FSENSOR_DIDNT_SWITCH_ON, ProgressCode::ERRWaitingForUser));

    ClearButtons(tc);
}

void ToolChangeFailLoadToFindaRightBtn(logic::ToolChange &tc, uint8_t toSlot) {
    // now waiting for user input - do not press anything
    PressButtonAndDebounce(tc, mb::Right);

    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InSelector, mi::Idler::IdleSlotIndex(), toSlot, false, false, ml::off, ml::blink0, ErrorCode::FINDA_DIDNT_SWITCH_ON, ProgressCode::ERRWaitingForUser));

    ClearButtons(tc);
}

TEST_CASE("tool_change::load_fail_FINDA_resolve_btnL", "[tool_change]") {
    logic::ToolChange tc;
    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount; ++toSlot) {
            if (fromSlot != toSlot) {
                ToolChangeFailLoadToFinda(tc, fromSlot, toSlot);
                ToolChangeFailLoadToFindaLeftBtn(tc, toSlot);
            }
        }
    }
}

TEST_CASE("tool_change::load_fail_FINDA_resolve_btnM", "[tool_change]") {
    logic::ToolChange tc;
    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount; ++toSlot) {
            if (fromSlot != toSlot) {
                ToolChangeFailLoadToFinda(tc, fromSlot, toSlot);
                ToolChangeFailLoadToFindaMiddleBtn(tc, toSlot);
            }
        }
    }
}

TEST_CASE("tool_change::load_fail_FINDA_resolve_btnR_FINDA_FSensor", "[tool_change]") {
    logic::ToolChange tc;
    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount; ++toSlot) {
            if (fromSlot != toSlot) {
                ToolChangeFailLoadToFinda(tc, fromSlot, toSlot);
                ToolChangeFailLoadToFindaRightBtnFINDA_FSensor(tc, toSlot);
            }
        }
    }
}

TEST_CASE("tool_change::load_fail_FINDA_resolve_btnR_FINDA", "[tool_change]") {
    logic::ToolChange tc;
    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount; ++toSlot) {
            if (fromSlot != toSlot) {
                ToolChangeFailLoadToFinda(tc, fromSlot, toSlot);
                ToolChangeFailLoadToFindaRightBtnFINDA(tc, toSlot);
            }
        }
    }
}

void ToolChangeFailFSensor(logic::ToolChange &tc, uint8_t fromSlot, uint8_t toSlot) {
    using namespace std::placeholders;
    ForceReinitAllAutomata();

    SetFINDAStateAndDebounce(true);
    mfs::fsensor.ProcessMessage(true);
    EnsureActiveSlotIndex(fromSlot, mg::FilamentLoadState::InNozzle);

    // restart the automaton
    tc.Reset(toSlot);

    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InNozzle, mi::idler.IdleSlotIndex(), fromSlot, true, true, ml::off, ml::off, ErrorCode::RUNNING, ProgressCode::UnloadingFilament));
    // simulate unload to finda but fail the fsensor test
    REQUIRE(WhileCondition(tc, std::bind(SimulateUnloadToFINDA, _1, 500'000, 10'000), 200'000));
    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InSelector, mi::idler.IdleSlotIndex(), fromSlot, false, false, ml::off, ml::blink0, ErrorCode::FSENSOR_DIDNT_SWITCH_OFF, ProgressCode::UnloadingFilament));
    REQUIRE(tc.unl.State() == ProgressCode::ERRWaitingForUser);
}

void ToolChangeFailFSensorMiddleBtn(logic::ToolChange &tc, uint8_t fromSlot, uint8_t toSlot) {
    using namespace std::placeholders;

    // user pulls filament out from the fsensor and presses Retry
    mfs::fsensor.ProcessMessage(false);
    PressButtonAndDebounce(tc, mb::Middle);
    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InSelector, mi::idler.IdleSlotIndex(), fromSlot, false, false, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::UnloadingFilament));
    REQUIRE(tc.unl.State() == ProgressCode::FeedingToFinda); // MMU must find out where the filament is FS is OFF, FINDA is OFF

    // both movables should have their homing flag invalidated
    REQUIRE_FALSE(mi::idler.HomingValid());
    REQUIRE_FALSE(ms::selector.HomingValid());

    // make FINDA trigger - Idler will rehome in this step, Selector must remain at its place
    SimulateIdlerHoming(tc);
    REQUIRE(mi::idler.HomingValid());
    REQUIRE_FALSE(ms::selector.HomingValid());
    // now trigger the FINDA
    REQUIRE(WhileCondition(tc, std::bind(SimulateFeedToFINDA, _1, 100), 5000));
    REQUIRE(VerifyState(tc, mg::FilamentLoadState::InSelector, fromSlot, fromSlot, true, true, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::UnloadingFilament));
    REQUIRE(tc.unl.State() == ProgressCode::RetractingFromFinda);

    // make FINDA switch off
    REQUIRE(WhileCondition(tc, std::bind(SimulateRetractFromFINDA, _1, 100), 5000));
    REQUIRE(WhileCondition(
        tc, [&](uint32_t) { return tc.unl.State() == ProgressCode::RetractingFromFinda; }, 50000));

    // Selector will start rehoming at this stage - that was the error this test was to find
    REQUIRE(VerifyState(tc, mg::FilamentLoadState::AtPulley, fromSlot, config::toolCount, false, true, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::UnloadingFilament));
    REQUIRE(tc.unl.State() == ProgressCode::DisengagingIdler);
    SimulateSelectorHoming(tc);

    // Idler has probably engaged meanwhile, ignore its position check
    REQUIRE(WhileTopState(tc, ProgressCode::UnloadingFilament, 50000));
    REQUIRE(VerifyState2(tc, mg::FilamentLoadState::AtPulley, config::toolCount, fromSlot, false, false, toSlot, ml::blink0, ml::off, ErrorCode::RUNNING, ProgressCode::FeedingToFinda));

    // after that, perform a normal load
    FeedingToFinda(tc, toSlot, 100);
    FeedingToBondtech(tc, toSlot);
    CheckFinishedCorrectly(tc, toSlot);
}

TEST_CASE("tool_change::load_fail_FSensor_resolve_btnM", "[tool_change]") {
    logic::ToolChange tc;
    for (uint8_t fromSlot = 0; fromSlot < config::toolCount; ++fromSlot) {
        for (uint8_t toSlot = 0; toSlot < config::toolCount; ++toSlot) {
            if (fromSlot != toSlot) {
                ToolChangeFailFSensor(tc, fromSlot, toSlot);
                ToolChangeFailFSensorMiddleBtn(tc, fromSlot, toSlot);
            }
        }
    }
}
