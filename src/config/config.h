#pragma once
#include <stdint.h>

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
static constexpr const uint8_t findaADCIndex = 6; ///< ADC index of FINDA input
static constexpr const uint16_t findaADCDecisionLevel = 512; ///< ADC decision level when a FINDA is considered pressed/not pressed

// Buttons setup
static constexpr const uint8_t buttonCount = 3; ///< number of buttons currently supported
static constexpr const uint16_t buttonsDebounceMs = 100;
static constexpr const uint16_t buttonADCLimits[buttonCount][2] = { { 0, 50 }, { 80, 100 }, { 160, 180 } };
static constexpr const uint8_t buttonsADCIndex = 5; ///< ADC index of buttons input

} // namespace config
