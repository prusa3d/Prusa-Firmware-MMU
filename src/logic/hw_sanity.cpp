/// @file hw_sanity.cpp
#include "hw_sanity.h"
#include "../modules/globals.h"
#include "../modules/motion.h"
#include "../modules/leds.h"
#include "../modules/timebase.h"
#include "config/axis.h"

namespace logic {

// Copy-pasta from command_base, they're inline
// so this shouldn't affect code size.
inline ErrorCode &operator|=(ErrorCode &a, ErrorCode b) {
    return a = (ErrorCode)((uint16_t)a | (uint16_t)b);
}

using Axis = config::Axis;
using TMC2130 = hal::tmc2130::TMC2130;

static constexpr uint8_t LED_WAIT_MS = 50U;
static constexpr uint8_t TEST_PASSES = 6U;
static_assert(TEST_PASSES < 32); // Would overflow counters

HWSanity hwSanity;

uint8_t HWSanity::test_step = 0;
uint8_t HWSanity::fault_masks[] = { 0 };
uint16_t HWSanity::wait_start = 0;
Axis HWSanity::axis;
ProgressCode HWSanity::next_state = ProgressCode::HWTestBegin;

bool HWSanity::Reset(uint8_t param) {
    state = ProgressCode::HWTestBegin;
    error = ErrorCode::RUNNING;
    axis = config::Axis::Idler;
    fault_masks[0] = 0;
    fault_masks[1] = 0;
    fault_masks[2] = 0;
    return true;
}

enum pin_bits {
    BIT_STEP = 0b001,
    BIT_DIR = 0b010,
    BIT_ENA = 0b100,
};

bool HWSanity::StepInner() {
    switch (state) {
    case ProgressCode::HWTestBegin:
        //auto& driver = mm::motion.DriverForAxis(config::Axis::Pulley);
        test_step = 0;
        // Todo - set TOFF so the output bridge is disabled.
        state = ProgressCode::HWTestIdler;
        break;
    case ProgressCode::HWTestIdler:
        axis = config::Axis::Idler;
        ml::leds.SetPairButOffOthers(5, ml::on, ml::off);
        state = ProgressCode::HWTestExec;
        next_state = ProgressCode::HWTestSelector;
        break;
    case ProgressCode::HWTestSelector:
        axis = config::Axis::Selector;
        ml::leds.SetPairButOffOthers(5, ml::off, ml::on);
        state = ProgressCode::HWTestExec;
        next_state = ProgressCode::HWTestPulley;
        break;
    case ProgressCode::HWTestPulley:
        axis = config::Axis::Pulley;
        ml::leds.SetPairButOffOthers(5, ml::on, ml::on);
        state = ProgressCode::HWTestExec;
        next_state = ProgressCode::HWTestCleanup;
        break;
    // The main test loop for a given axis.
    case ProgressCode::HWTestDisplay:
        // Hold for a few ms while we display the last step result.
        if (!mt::timebase.Elapsed(wait_start, LED_WAIT_MS)) {
            break;
        } else {
            state = ProgressCode::HWTestExec;
        }
        /* FALLTHRU */
    case ProgressCode::HWTestExec: {
        auto driver = mm::motion.DriverForAxis(axis);
        auto params = mm::axisParams[axis].params;
        if (test_step < (TEST_PASSES * 8)) // 8 combos per axis
        {
            uint8_t set_state = test_step % 8;
            // The order of the bits here is roughly the same as that of IOIN.
            driver.SetStep(params, set_state & BIT_STEP);
            driver.SetDir(params, set_state & BIT_DIR);
            driver.SetEnabled(params, set_state & BIT_ENA);
            uint16_t drv_ioin = driver.ReadRegister(params, TMC2130::Registers::IOIN);
            // Compose IOIN to look like set_state.
            drv_ioin = (drv_ioin & 0b11) | ((drv_ioin & 0b10000) >> 2);
            uint8_t bit_errs = (drv_ioin ^ set_state);
            // Set the LEDs. Note RED is index 0 in the enum, so we want  the expression FALSE if there's an error.
            ml::leds.SetMode(0, static_cast<ml::Color>((bit_errs & BIT_STEP) == 0), ml::on);
            ml::leds.SetMode(1, static_cast<ml::Color>((bit_errs & BIT_DIR) == 0), ml::on);
            ml::leds.SetMode(2, static_cast<ml::Color>((bit_errs & BIT_ENA) == 0), ml::on);
            // Capture the error for later.
            fault_masks[axis] |= bit_errs;
            // Enter the wait state:
            wait_start = mt::timebase.Millis();
            state = ProgressCode::HWTestDisplay;
            // Next iteration.
            test_step++;
        } else {
            // This pass is complete. Move on to the next motor or cleanup.
            test_step = 0;
            state = next_state;
        }
    } break;
    case ProgressCode::HWTestCleanup:
        if (fault_masks[0] || fault_masks[1] || fault_masks[2]) {
            // error, display it and return the code.
            state = ProgressCode::ErrHwTestFailed;
            error = ErrorCode::TMC_PINS_UNRELIABLE;
            if (fault_masks[Axis::Idler]) {
                error |= ErrorCode::TMC_IDLER_BIT;
            }
            if (fault_masks[Axis::Pulley]) {
                error |= ErrorCode::TMC_PULLEY_BIT;
            }
            if (fault_masks[Axis::Selector]) {
                error |= ErrorCode::TMC_SELECTOR_BIT;
            }
            return true;
        } else {
            //TODO: Re-enable TOFF here
            FinishedOK();
        }
    case ProgressCode::OK:
        return true;
    default: // we got into an unhandled state, better report it
        state = ProgressCode::ERRInternal;
        error = ErrorCode::INTERNAL;
        return true;
    }
    return false;
}

} // namespace logic
