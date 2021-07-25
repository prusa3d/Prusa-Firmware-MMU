#pragma once
#include <stdint.h>
#include "../unit.h"

namespace config {

using namespace unit;

/// Axis configuration data
struct AxisConfig {
    bool dirOn; ///< direction ON state (for inversion)
    bool vSense; ///< vSense scaling
    uint8_t iRun; ///< running current
    uint8_t iHold; ///< holding current
    bool stealth; ///< Default to Stealth mode
    uint8_t uSteps; ///< microstepping [1-256]
    long double stepsPerUnit; ///< steps per unit
};

/// List of available axes
enum Axis : uint8_t {
    Pulley,
    Selector,
    Idler,
    _Axis_Last = Idler
};

/// Number of available axes
static constexpr uint8_t NUM_AXIS = Axis::_Axis_Last + 1;

/// Phisical limits for an axis
template <UnitBase B>
struct AxisLimits {
    static constexpr UnitBase base = B;
    Unit<long double, B, Lenght> lenght; ///< Longest move that can be performed by the axis
    Unit<long double, B, Speed> jerk; ///< Maximum jerk for the axis
    Unit<long double, B, Accel> accel; ///< Maximum acceleration for the axis
};

typedef AxisLimits<Millimeter> PulleyLimits; ///< Pulley axis limits
typedef AxisLimits<Millimeter> SelectorLimits; ///< Selector axis limits
typedef AxisLimits<Degree> IdlerLimits; ///< Idler axis limits

} // namespace config
