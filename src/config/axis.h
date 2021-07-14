#pragma once
#include <stdint.h>

namespace config {

/// Axis configuration data
struct AxisConfig {
    bool dirOn; ///< direction ON state (for inversion)
    uint8_t uSteps; ///< microstepping [1-32]
    bool vSense; ///< vSense scaling
    uint8_t iRun; ///< running current
    uint8_t iHold; ///< holding current
    uint16_t accel; ///< Acceleration (unit/s^2)
    uint16_t jerk; ///< Jerk (unit/s)
    bool stealth; ///< Default to Stealth mode
};

} // namespace config
