#pragma once
#include <stdint.h>
#include "axis.h"

/// Wrangler for assorted compile-time configuration and constants.
namespace config {

static constexpr const uint8_t toolCount = 5U; ///< Max number of extruders/tools/slots

/// Absolute positions for Idler's slots: 0-4 are the real ones, the 5th index is the idle position
static constexpr U_deg idlerSlotPositions[toolCount + 1] = {
    45.0_deg,
    2 * 45.0_deg,
    3 * 45.0_deg,
    4 * 45.0_deg,
    5 * 45.0_deg,
    0.0_deg
};

static constexpr U_deg_s idlerFeedrate = 1000._deg_s;

// Selector's setup

/// slots 0-4 are the real ones, the 5th is the farthest parking positions
static constexpr U_mm selectorSlotPositions[toolCount + 1] = {
    20.0_mm,
    20.0_mm + 14.0_mm,
    20.0_mm + 2 * 14.0_mm,
    20.0_mm + 3 * 14.0_mm,
    20.0_mm + 4 * 14.0_mm,
    20.0_mm + 5 * 14.0_mm
};

static constexpr U_mm_s selectorFeedrate = 1000._mm_s;

// Printer's filament sensor setup
static constexpr const uint16_t fsensorDebounceMs = 10;

// LEDS
/// The complete period of LED's blinking (i.e. ON and OFF together)
static constexpr uint16_t ledBlinkPeriodMs = 1000U;

// FINDA setup
static constexpr const uint16_t findaDebounceMs = 100;
static constexpr const uint8_t findaADCIndex = 6; ///< ADC index of FINDA input
static constexpr const uint16_t findaADCDecisionLevel = 512; ///< ADC decision level when a FINDA is considered pressed/not pressed

// Buttons setup
static constexpr const uint8_t buttonCount = 3; ///< number of buttons currently supported
static constexpr const uint16_t buttonsDebounceMs = 100;
static constexpr const uint16_t buttonADCLimits[buttonCount][2] = { { 0, 50 }, { 80, 100 }, { 160, 180 } };
static constexpr const uint8_t buttonsADCIndex = 5; ///< ADC index of buttons input

// Motion and planning

/// Do not plan moves equal or shorter than the requested steps
static constexpr uint8_t dropSegments = 0;

/// Max step frequency 40KHz
static constexpr uint16_t maxStepFrequency = 40000;

/// Minimum stepping rate 120Hz
static constexpr uint16_t minStepRate = 120;

/// Size for the motion planner block buffer size
static constexpr uint8_t blockBufferSize = 2;

/// Step timer frequency divider (F = F_CPU / divider)
static constexpr uint8_t stepTimerFrequencyDivider = 8;

/// Smallest stepping ISR scheduling slice (T = F_CPU / divider * quantum)
/// 16 = 8us (25us is the max frequency interval per maxStepFrequency)
static constexpr uint8_t stepTimerQuantum = 16;

/// Pulley axis configuration
static constexpr AxisConfig pulley = {
    .dirOn = false,
    .mRes = MRes_2,
    .vSense = true,
    .iRun = 30,
    .iHold = 1,
    .stealth = false,
    .stepsPerUnit = 161.3,
};

/// Pulley motion limits
static constexpr PulleyLimits pulleyLimits = {
    .lenght = 1000.0_mm, // TODO
    .jerk = 4.0_mm_s,
    .accel = 800.0_mm_s2,
};

/// Selector configuration
static constexpr AxisConfig selector = {
    .dirOn = true,
    .mRes = MRes_2,
    .vSense = false,
    .iRun = 17,
    .iHold = 5,
    .stealth = false,
    .stepsPerUnit = (200 * 2 / 8.),
};

/// Selector motion limits
static constexpr SelectorLimits selectorLimits = {
    .lenght = 75.0_mm,
    .jerk = 1.0_mm_s,
    .accel = 200.0_mm_s2,
};

/// Idler configuration
static constexpr AxisConfig idler = {
    .dirOn = true,
    .mRes = MRes_16,
    .vSense = false,
    .iRun = 23,
    .iHold = 11,
    .stealth = false,
    .stepsPerUnit = (200 * 16 / 360.),
};

/// Idler motion limits
static constexpr IdlerLimits idlerLimits = {
    .lenght = 270.0_deg,
    .jerk = 0.1_deg_s,
    .accel = 10.0_deg_s2,
};

/// Max retries of FeedToBondtech used in LoadFilament
static constexpr uint8_t feedToBondtechMaxRetries = 2;

// TMC2130 setup

static constexpr int8_t tmc2130_sg_thrs = 3; // @todo 7bit two's complement for the sg_thrs
static_assert(tmc2130_sg_thrs >= -64 && tmc2130_sg_thrs <= 63, "tmc2130_sg_thrs out of range");

static constexpr uint32_t tmc2130_coolStepThreshold = 400; ///< step-based 20bit uint
static_assert(tmc2130_coolStepThreshold <= 0xfffff, "tmc2130_coolStepThreshold out of range");

static constexpr uint32_t tmc2130_PWM_AMPL = 240;
static_assert(tmc2130_PWM_AMPL <= 255, "tmc2130_PWM_AMPL out of range");

static constexpr uint32_t tmc2130_PWM_GRAD = 4;
static_assert(tmc2130_PWM_GRAD <= 255, "tmc2130_PWM_GRAD out of range");

static constexpr uint32_t tmc2130_PWM_FREQ = 2;
static_assert(tmc2130_PWM_FREQ <= 3, "tmc2130_PWM_GRAD out of range");

static constexpr uint32_t tmc2130_PWM_AUTOSCALE = 1;
static_assert(tmc2130_PWM_AUTOSCALE <= 1, "tmc2130_PWM_AUTOSCALE out of range");

} // namespace config
