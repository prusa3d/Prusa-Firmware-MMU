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
    float scale; ///< Scaling unit (unit/uStepsMaxRes)
    float accel; ///< Acceleration (unit/s^2)
    float jerk; ///< Jerk (unit/s)
};

} // namespace config
