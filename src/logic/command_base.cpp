#include "command_base.h"
#include "../modules/idler.h"
#include "../modules/selector.h"
#include "../modules/motion.h"
#include "../modules/leds.h"

namespace logic {

inline ErrorCode &operator|=(ErrorCode &a, ErrorCode b) {
    return a = (ErrorCode)((uint16_t)a | (uint16_t)b);
}

static ErrorCode TMC2130ToErrorCode(const hal::tmc2130::TMC2130 &tmc, uint8_t tmcIndex) {
    ErrorCode e = ErrorCode::RUNNING;

    if (tmc.GetErrorFlags().reset_flag) {
        e |= ErrorCode::TMC_RESET;
    }
    if (tmc.GetErrorFlags().uv_cp) {
        e |= ErrorCode::TMC_UNDERVOLTAGE_ON_CHARGE_PUMP;
    }
    if (tmc.GetErrorFlags().s2g) {
        e |= ErrorCode::TMC_SHORT_TO_GROUND;
    }
    if (tmc.GetErrorFlags().otpw) {
        e |= ErrorCode::TMC_OVER_TEMPERATURE_WARN;
    }
    if (tmc.GetErrorFlags().ot) {
        e |= ErrorCode::TMC_OVER_TEMPERATURE_ERROR;
    }

    if (e != ErrorCode::RUNNING) {
        switch (tmcIndex) {
        case config::Axis::Pulley:
            e |= ErrorCode::TMC_PULLEY_BIT;
            break;
        case config::Axis::Selector:
            e |= ErrorCode::TMC_SELECTOR_BIT;
            break;
        case config::Axis::Idler:
            e |= ErrorCode::TMC_IDLER_BIT;
            break;
        default:
            break;
        }
    }

    return e;
}

bool CommandBase::Step() {
    ErrorCode tmcErr = ErrorCode::RUNNING;
    // check the global HW errors - may be we should avoid the modules layer and check for the HAL layer errors directly
    if (mi::idler.State() == mi::Idler::Failed) {
        state = ProgressCode::ERRTMCFailed;
        tmcErr |= TMC2130ToErrorCode(mm::motion.DriverForAxis(mm::Axis::Idler), mm::Axis::Idler);
    }
    if (ms::selector.State() == ms::Selector::Failed) {
        state = ProgressCode::ERRTMCFailed;
        tmcErr |= TMC2130ToErrorCode(mm::motion.DriverForAxis(mm::Axis::Selector), mm::Axis::Selector);
    }
    // may be we should model the Pulley as well...
    //    if (ms::selector.State() == ms::Selector::Failed) {
    //        state = ProgressCode::ERRTMCFailed;
    //        error |= TMC2130ToErrorCode(mm::motion.DriverForAxis(mm::Axis::Selector), mm::Axis::Selector);
    //        return true; // the HW error prevents us from continuing with the state machine - the MMU must be restarted/fixed before continuing
    //    }

    // @@TODO not sure how to prevent losing the previously accumulated error ... or do I really need to do it?
    // May be the TMC error word just gets updated with new flags as the motion proceeds
    // And how about the logical errors like FINDA_DIDNT_SWITCH_ON?
    if (tmcErr != ErrorCode::RUNNING) {
        error |= tmcErr;
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

bool CommandBase::CheckToolIndex(uint8_t index) {
    if (index >= config::toolCount) {
        error = ErrorCode::INVALID_TOOL;
        return false;
    } else {
        error = ErrorCode::OK;
        return true;
    }
}

} // namespace logic
