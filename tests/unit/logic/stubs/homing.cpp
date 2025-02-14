#include "catch2/catch_test_macros.hpp"
#include "homing.h"
#include "main_loop_stub.h"

#include "../../../../src/modules/buttons.h"
#include "../../../../src/modules/idler.h"
#include "../../../../src/modules/motion.h"
#include "../../../../src/modules/selector.h"

#include "../stubs/stub_motion.h"

void SimulateIdlerAndSelectorHoming(logic::CommandBase &cb) {
#if 0
    // do 5 steps until we trigger the simulated StallGuard
    for (uint8_t i = 0; i < 5; ++i) {
        main_loop();
        cb.Step();
    }

    mm::TriggerStallGuard(mm::Selector);
    mm::TriggerStallGuard(mm::Idler);
    main_loop();
    cb.Step();
    mm::motion.StallGuardReset(mm::Selector);
    mm::motion.StallGuardReset(mm::Idler);

    // now do a correct amount of steps of each axis towards the other end
    uint32_t idlerSteps = mm::unitToSteps<mm::I_pos_t>(config::idlerLimits.lenght);
    uint32_t selectorSteps = mm::unitToSteps<mm::S_pos_t>(config::selectorLimits.lenght);
    uint32_t maxSteps = std::max(idlerSteps, selectorSteps) + 1;

    for (uint32_t i = 0; i < maxSteps; ++i) {
        main_loop();
        cb.Step();

        if (i == idlerSteps) {
            mm::TriggerStallGuard(mm::Idler);
        } else {
            mm::motion.StallGuardReset(mm::Idler);
        }
        if (i == selectorSteps) {
            mm::TriggerStallGuard(mm::Selector);
        } else {
            mm::motion.StallGuardReset(mm::Selector);
        }
    }

    // now the Selector and Idler shall perform a move into their parking positions
    while (ms::selector.State() != mm::MovableBase::Ready || mi::idler.State() != mm::MovableBase::Ready) {
        main_loop();
        cb.Step();
    }
#else
    // sadly, it looks like we need to separate homing of idler and selector due to electrical reasons
    SimulateIdlerHoming(cb);
    SimulateSelectorHoming(cb);
#endif
}

void SimulateIdlerHoming(logic::CommandBase &cb) {
    uint32_t idlerStepsFwd = mm::unitToSteps<mm::I_pos_t>(config::idlerLimits.lenght - 5.0_deg);

    // Sometimes the initial idler state is Ready. Let's wait for the firmware to start
    // homing.
    REQUIRE(WhileCondition(
        cb,
        [&](uint32_t) { return mi::idler.State() == mm::MovableBase::Ready; },
        5000));

    // At this point the idler should always be homing forward.
    REQUIRE((int)mi::idler.State() == (int)mm::MovableBase::HomeForward);

    // Simulate the idler steps in one direction (forward)
    for (uint32_t i = 0; i < idlerStepsFwd; ++i) {
        main_loop();
        cb.Step();
    }

    mm::TriggerStallGuard(mm::Idler);
    main_loop();
    cb.Step();
    mm::motion.StallGuardReset(mm::Idler);

    REQUIRE((int)mi::idler.State() == (int)mm::MovableBase::HomeBack);

    // now do a correct amount of steps of each axis towards the other end
    uint32_t idlerSteps = mm::unitToSteps<mm::I_pos_t>(config::idlerLimits.lenght);
    uint32_t maxSteps = idlerSteps + 1;

    for (uint32_t i = 0; i < maxSteps; ++i) {
        main_loop();
        cb.Step();

        if (i == idlerSteps) {
            mm::TriggerStallGuard(mm::Idler);
        } else {
            mm::motion.StallGuardReset(mm::Idler);
        }
    }

    // If the homing has failed, the axis length was too short.
    REQUIRE(!((mi::idler.State() & mm::MovableBase::HomingFailed) == mm::MovableBase::HomingFailed));
}

void SimulateIdlerWaitForHomingValid(logic::CommandBase &cb) {
    // Wait for the HomingValid flag to be set
    while (!mi::idler.HomingValid()) {
        main_loop();
        cb.Step();
    }
}

void SimulateIdlerMoveToParkingPosition(logic::CommandBase &cb) {
    // now the Idler shall perform a move into their parking positions
    while (mi::idler.State() != mm::MovableBase::Ready) {
        main_loop();
        cb.Step();
    }
}

void SimulateSelectorHoming(logic::CommandBase &cb) {
    // do 5 steps until we trigger the simulated StallGuard
    for (uint8_t i = 0; i < 5; ++i) {
        main_loop();
        cb.Step();
    }

    mm::TriggerStallGuard(mm::Selector);
    main_loop();
    cb.Step();
    mm::motion.StallGuardReset(mm::Selector);

    // now do a correct amount of steps of each axis towards the other end
    uint32_t selectorSteps = mm::unitToSteps<mm::S_pos_t>(config::selectorLimits.lenght) + 1;
    uint32_t maxSteps = selectorSteps + 1;

    for (uint32_t i = 0; i < maxSteps; ++i) {
        main_loop();
        cb.Step();

        if (i == selectorSteps) {
            mm::TriggerStallGuard(mm::Selector);
        } else {
            mm::motion.StallGuardReset(mm::Selector);
        }
    }
}

void SimulateSelectorWaitForHomingValid(logic::CommandBase &cb) {
    // Wait for the HomingValid flag to be set
    while (!ms::selector.HomingValid()) {
        main_loop();
        cb.Step();
    }
}

void SimulateSelectorWaitForReadyState(logic::CommandBase &cb) {
    // now the Selector shall perform a move into their parking positions
    while (ms::selector.State() != mm::MovableBase::Ready) {
        main_loop();
        cb.Step();
    }
}

void SimulateSelectorAndIdlerWaitForReadyState(logic::CommandBase &cb) {
    while (ms::selector.State() != ms::Selector::Ready && mi::idler.State() != mi::Idler::Ready) {
        main_loop();
        cb.Step();
    }
}

bool SimulateFailedHomeSelectorPostfix(logic::CommandBase &cb) {
    if (!WhileTopState(cb, ProgressCode::Homing, 5))
        return false;
    if (cb.Error() != ErrorCode::HOMING_SELECTOR_FAILED)
        return false;
    if (cb.State() != ProgressCode::ERRWaitingForUser)
        return false;
    if (mm::motion.Enabled(mm::Selector))
        return false;

    // do a few steps before pushing the button
    WhileTopState(cb, ProgressCode::ERRWaitingForUser, 5);

    if (mm::motion.Enabled(mm::Selector))
        return false;

    PressButtonAndDebounce(cb, mb::Middle, false);

    // it shall start homing again
    if (cb.Error() != ErrorCode::RUNNING)
        return false;
    if (cb.State() != ProgressCode::Homing)
        return false;
    if (ms::selector.HomingValid())
        return false;
    if (!mm::motion.Enabled(mm::Selector))
        return false;

    ClearButtons(cb);

    return true;
}

bool SimulateFailedHomeFirstTime(logic::CommandBase &cb) {
    REQUIRE(!mi::idler.HomingValid());
    REQUIRE(!ms::selector.HomingValid());

    // Idler homing is successful
    SimulateIdlerHoming(cb);
    SimulateIdlerWaitForHomingValid(cb);

    // Selector homes once the idler homing is valid.
    REQUIRE(mi::idler.HomingValid());
    REQUIRE(!ms::selector.HomingValid());

    // The selector will only rehome once the idler homing is valid. At that moment
    // the state will change to HomeForward.
    REQUIRE(WhileCondition(
        cb,
        [&](uint32_t) { return ms::selector.State() != mm::MovableBase::HomeForward; },
        5000));

    constexpr uint32_t selectorSteps = mm::unitToSteps<mm::S_pos_t>(config::selectorLimits.lenght) + 1;
    {
        // do 5 steps until we trigger the simulated StallGuard
        for (uint32_t i = 0; i < selectorSteps; ++i) {
            main_loop();
            cb.Step();
        }

        mm::TriggerStallGuard(mm::Selector);
        main_loop();
        cb.Step();
        mm::motion.StallGuardReset(mm::Selector);
    }

    // now do LESS steps than expected to simulate something is blocking the selector
    constexpr uint32_t selectorTriggerShort = selectorSteps / 2;
    constexpr uint32_t maxSteps = selectorTriggerShort + 1;
    {
        for (uint32_t i = 0; i < maxSteps; ++i) {
            main_loop();
            cb.Step();

            if (i == selectorTriggerShort) {
                mm::TriggerStallGuard(mm::Selector);
            } else {
                mm::motion.StallGuardReset(mm::Selector);
            }
        }

        while (!(ms::selector.State() & mm::MovableBase::OnHold)) {
            main_loop();
            cb.Step();
        }
    }

    return SimulateFailedHomeSelectorPostfix(cb);
}

bool SimulateFailedHomeSelectorRepeated(logic::CommandBase &cb) {
    // we leave Idler aside in this case
    if (ms::selector.HomingValid())
        return false;

    {
        // do 5 steps until we trigger the simulated StallGuard
        for (uint8_t i = 0; i < 5; ++i) {
            main_loop();
            cb.Step();
        }

        mm::TriggerStallGuard(mm::Selector);
        main_loop();
        cb.Step();
        mm::motion.StallGuardReset(mm::Selector);
    }
    uint32_t selectorSteps = mm::unitToSteps<mm::S_pos_t>(config::selectorLimits.lenght) + 1;
    uint32_t selectorTriggerShort = selectorSteps / 2;
    uint32_t maxSteps = selectorTriggerShort + 1;
    {
        for (uint32_t i = 0; i < maxSteps; ++i) {
            main_loop();
            cb.Step();

            if (i == selectorTriggerShort) {
                mm::TriggerStallGuard(mm::Selector);
            } else {
                mm::motion.StallGuardReset(mm::Selector);
            }
        }

        while (!(ms::selector.State() & mm::MovableBase::OnHold)) {
            main_loop();
            cb.Step();
        }
    }

    return SimulateFailedHomeSelectorPostfix(cb);
}
