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
static constexpr const uint8_t findaADCIndex = 1; ///< ADC index of FINDA input
static constexpr const uint16_t findaADCDecisionLevel = 512; ///< ADC decision level when a FINDA is considered pressed/not pressed

// Buttons setup
static constexpr const uint16_t buttonsDebounceMs = 100;
static constexpr const uint16_t button0ADCMin = 0;
static constexpr const uint16_t button0ADCMax = 10;
static constexpr const uint16_t button1ADCMin = 320;
static constexpr const uint16_t button1ADCMax = 360;
static constexpr const uint16_t button2ADCMin = 500;
static constexpr const uint16_t button2ADCMax = 530;
static constexpr const uint8_t buttonsADCIndex = 0; ///< ADC index of buttons input

} // namespace config
