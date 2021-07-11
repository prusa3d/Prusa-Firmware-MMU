#pragma once
#include <stdint.h>
#include "axis.h"

/// Wrangler for assorted compile-time configuration and constants.
namespace config {

static constexpr const uint8_t toolCount = 5U; ///< Max number of extruders/tools/slots

// Idler's setup
static constexpr uint16_t idlerSlotPositions[toolCount + 1] = { 1, 2, 3, 4, 5, 0 }; ///< slots 0-4 are the real ones, the 5th is the idle position

// Selector's setup
static constexpr uint16_t selectorSlotPositions[toolCount + 1] = { 1, 2, 3, 4, 5, 6 }; ///< slots 0-4 are the real ones, the 5th is the farthest parking positions

// Printer's filament sensor setup
static constexpr const uint16_t fsensorDebounceMs = 10;

// FINDA setup
static constexpr const uint16_t findaDebounceMs = 100;
static constexpr const uint8_t findaADCIndex = 1; ///< ADC index of FINDA input
static constexpr const uint16_t findaADCDecisionLevel = 512; ///< ADC decision level when a FINDA is considered pressed/not pressed

// Buttons setup
static constexpr const uint8_t buttonCount = 3; ///< number of buttons currently supported
static constexpr const uint16_t buttonsDebounceMs = 100;
static constexpr const uint16_t buttonADCLimits[buttonCount][2] = { { 0, 10 }, { 320, 360 }, { 500, 530 } };
static constexpr const uint8_t buttonsADCIndex = 0; ///< ADC index of buttons input

/// Maximum microstepping resolution. This defines the effective unit of
/// the step intevals on the motion API, independently of the selected
/// microstepping interval.
static constexpr uint8_t uStepMaxRes = 32;

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

/// Idler configuration
static constexpr AxisConfig idler = {
    .dirOn = true,
    .uSteps = 16,
    .vSense = false,
    .iRun = 20,
    .iHold = 20,
    .accel = 100,
    .jerk = 10,
    .stealth = false,
};

/// Pulley configuration
static constexpr AxisConfig pulley = {
    .dirOn = true,
    .uSteps = 16,
    .vSense = false,
    .iRun = 20,
    .iHold = 20,
    .accel = 100,
    .jerk = 10,
    .stealth = false,
};

/// Selector configuration
static constexpr AxisConfig selector = {
    .dirOn = true,
    .uSteps = 16,
    .vSense = false,
    .iRun = 20,
    .iHold = 20,
    .accel = 100,
    .jerk = 1,
    .stealth = false
};

} // namespace config
