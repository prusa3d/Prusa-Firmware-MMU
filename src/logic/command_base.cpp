/// @file command_base.cpp
#include "command_base.h"
#include "../modules/globals.h"
#include "../modules/finda.h"
#include "../modules/fsensor.h"
#include "../modules/idler.h"
#include "../modules/pulley.h"
#include "../modules/selector.h"
#include "../modules/motion.h"
#include "../modules/leds.h"
#include "../modules/user_input.h"

namespace logic {

inline ErrorCode &operator|=(ErrorCode &a, ErrorCode b) {
    return a = (ErrorCode)((uint16_t)a | (uint16_t)b);
}

static ErrorCode TMC2130ToErrorCode(const hal::tmc2130::ErrorFlags &ef) {
    ErrorCode e = ErrorCode::RUNNING;

    if (ef.reset_flag) {
        e |= ErrorCode::TMC_RESET;
    }
    if (ef.uv_cp) {
        e |= ErrorCode::TMC_UNDERVOLTAGE_ON_CHARGE_PUMP;
    }
    if (ef.s2g) {
        e |= ErrorCode::TMC_SHORT_TO_GROUND;
    }
    if (ef.otpw) {
        e |= ErrorCode::TMC_OVER_TEMPERATURE_WARN;
    }
    if (ef.ot) {
        e |= ErrorCode::TMC_OVER_TEMPERATURE_ERROR;
    }

    return e;
}

static ErrorCode AddErrorAxisBit(ErrorCode ec, uint8_t tmcIndex) {
    switch (tmcIndex) {
    case config::Axis::Pulley:
        ec |= ErrorCode::TMC_PULLEY_BIT;
        break;
    case config::Axis::Selector:
        ec |= ErrorCode::TMC_SELECTOR_BIT;
        break;
    case config::Axis::Idler:
        ec |= ErrorCode::TMC_IDLER_BIT;
        break;
    default:
        break;
    }
    return ec;
}

ErrorCode CheckMovable(mm::MovableBase &m) {
    switch (m.State()) {
    case mm::MovableBase::TMCFailed:
        return AddErrorAxisBit(TMC2130ToErrorCode(m.TMCErrorFlags()), m.Axis());
    case mm::MovableBase::HomingFailed:
        return AddErrorAxisBit(ErrorCode::HOMING_FAILED, m.Axis());
    }
    return ErrorCode::RUNNING;
}

static inline ErrorCode WithoutAxisBits(ErrorCode ec) {
    return static_cast<ErrorCode>(
        static_cast<uint16_t>(ec)
        & (~(static_cast<uint16_t>(ErrorCode::TMC_SELECTOR_BIT)
            | static_cast<uint16_t>(ErrorCode::TMC_IDLER_BIT)
            | static_cast<uint16_t>(ErrorCode::TMC_PULLEY_BIT))));
}

bool CommandBase::WaitForOneModuleErrorRecovery(ErrorCode ec, modules::motion::MovableBase &m) {
    if (ec != ErrorCode::RUNNING) {
        if (stateBeforeModuleFailed == ProgressCode::OK) {
            // a new problem with the movable modules
            // @@TODO not sure how to prevent losing the previously accumulated error ... or do I really need to do it?
            // May be the TMC error word just gets updated with new flags as the motion proceeds
            stateBeforeModuleFailed = state;
            error = ec;
            state = ProgressCode::ERRWaitingForUser; // such a situation always requires user's attention -> let the printer display an error screen
            return true;
        } else {
            switch (state) {
            case ProgressCode::ERRWaitingForUser: // waiting for a recovery - mask axis bits:
                if (WithoutAxisBits(ec) == ErrorCode::HOMING_FAILED) {
                    // homing can be recovered
                    mui::Event ev = mui::userInput.ConsumeEvent();
                    if (ev == mui::Event::Middle) {
                        m.InvalidateHoming(); // @@TODO invalidate and force initiate a new homing attempt
                        state = ProgressCode::Homing;
                    }
                }
                // TMC errors cannot be recovered safely, waiting for power cycling the MMU
                return true;
            case ProgressCode::Homing:
                if (m.HomingValid()) {
                    // managed to recover from a homing problem
                    state = stateBeforeModuleFailed;
                    stateBeforeModuleFailed = ProgressCode::OK;
                    return false;
                }
                return true;
            default:
                return true; // no idea what to do in other states ... set internal fw error state?
            }
            return true;
        }
    }
    return false;
}

bool CommandBase::WaitForModulesErrorRecovery() {
    if (WaitForOneModuleErrorRecovery(CheckMovable(mi::idler), mi::idler))
        return true;

    if (WaitForOneModuleErrorRecovery(CheckMovable(ms::selector), ms::selector))
        return true;

    if (WaitForOneModuleErrorRecovery(CheckMovable(mpu::pulley), mpu::pulley))
        return true;

    return false;
}

bool CommandBase::Step() {
    if (WaitForModulesErrorRecovery()) {
        return true;
    }
    return StepInner();
}

void CommandBase::Panic(ErrorCode ec) {
    state = ProgressCode::ERRInternal;
    error = ec;
    for (uint8_t i = 0; i < config::toolCount; ++i) {
        ml::leds.SetMode(i, ml::green, ml::blink0);
        ml::leds.SetMode(i, ml::red, ml::blink0);
    }
}

void CommandBase::InvalidateHoming() {
    mi::idler.InvalidateHoming();
    ms::selector.InvalidateHoming();
}

void CommandBase::InvalidateHomingAndFilamentState() {
    InvalidateHoming();

    // reset the filament presence according to available sensor states
    bool fs = mfs::fsensor.Pressed();
    bool fi = mf::finda.Pressed();

    if (fs && fi) {
        mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::InNozzle);
    } else if (!fs && fi) {
        mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::InSelector);
    } else if (!fs && !fi) {
        mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::AtPulley);
    } else {
        // we can't say for sure - definitely an error in sensors or something is blocking them
        // let's assume there is a piece of filament present in the selector
        mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), mg::InSelector);
    }
}

bool CommandBase::CheckToolIndex(uint8_t index) {
    if (index >= config::toolCount) {
        error = ErrorCode::INVALID_TOOL;
        return false;
    } else {
        error = ErrorCode::OK;
        return true;
    }
}

void CommandBase::ErrDisengagingIdler() {
    if (!mi::idler.Engaged()) {
        state = ProgressCode::ERRWaitingForUser;
        mpu::pulley.Disable();
        mui::userInput.Clear(); // remove all buffered events if any just before we wait for some input
    }
}

void CommandBase::GoToErrDisengagingIdler(ErrorCode ec) {
    state = ProgressCode::ERRDisengagingIdler;
    error = ec;
    ml::leds.SetPairButOffOthers(mg::globals.ActiveSlot(), ml::off, ml::blink0);
    mi::idler.Disengage();
}

void CommandBase::GoToErrEngagingIdler() {
    state = ProgressCode::ERREngagingIdler;
    error = ErrorCode::RUNNING;
    mi::idler.Engage(mg::globals.ActiveSlot());
}

} // namespace logic
