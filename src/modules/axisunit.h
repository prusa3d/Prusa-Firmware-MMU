#pragma once
#include "../config/axis.h"
#include "pulse_gen.h"

namespace modules {
namespace motion {

// Import required types
using config::Axis;
using config::Idler;
using config::Pulley;
using config::Selector;

using config::Accel;
using config::Lenght;
using config::Speed;

using pulse_gen::pos_t;
using pulse_gen::steps_t;

/// Specialized axis unit type for compile-time conformability testing. Like config::Unit
/// this is done ensure unit quantities are not mixed between types, while also providing
/// convenience methods to convert from physical units to AxisUnits directly at compile.
///
/// Each axis unit type is separate for each axis, since the low-level count is not
/// directly comparable across axes. Quantities are normally defined through the
/// literar operators. Types and base axes are prefixed with a single letter identifier
/// for the axis: P=pulley, S=selector, I=idler.
///
/// P_pos_t pulley_position = 10.0_P_mm;
/// auto pulley_zero = 0.0_P_mm; // implicit type
/// P_speed_ pulley_feedrate = 30.0_P_mm_s;
/// I_pos_t idler_position = 15.0_I_deg;
/// pulley_position + idler_position; // compile time error
///
/// modules::motion::Motion.PlanMove (and related functions) support AxisUnit natively.
/// The low-level step count can be accessed when necessary through AxisUnit::v, which
/// should be avoided as it bypasses type checks.
template <typename T, Axis A, config::UnitType U>
struct AxisUnit {
    T v;

    typedef T type_t;
    typedef AxisUnit<T, A, U> self_t;

    constexpr self_t operator+(const self_t r) { return { v + r.v }; }
    constexpr self_t operator-(const self_t r) { return { v - r.v }; }
    constexpr self_t operator-() { return { -v }; }
    constexpr self_t operator*(const self_t r) { return { v * r.v }; }
    constexpr self_t operator/(const self_t r) { return { v / r.v }; }
};

typedef AxisUnit<pos_t, Pulley, Lenght> P_pos_t; ///< Pulley position type (steps)
typedef AxisUnit<steps_t, Pulley, Speed> P_speed_t; ///< Pulley speed type (steps/s)
typedef AxisUnit<steps_t, Pulley, Accel> P_accel_t; ///< Pulley acceleration type (steps/s2)

/// Convert a Unit to AxisUnit
template <typename T, typename U>
static constexpr T unitToAxisUnit(const long double stepsPerUnit, U v) {
    return { (typename T::type_t)(v.v * stepsPerUnit) };
}

static constexpr P_pos_t operator"" _P_mm(long double mm) {
    return { unitToAxisUnit<P_pos_t>(config::pulley.stepsPerUnit, config::U_mm { mm }) };
}

static constexpr P_speed_t operator"" _P_mm_s(long double mm_s) {
    return { unitToAxisUnit<P_speed_t>(config::pulley.stepsPerUnit, config::U_mm { mm_s }) };
}

static constexpr P_accel_t operator"" _P_mm_s2(long double mm_s2) {
    return { unitToAxisUnit<P_accel_t>(config::pulley.stepsPerUnit, config::U_mm_s2 { mm_s2 }) };
}

typedef AxisUnit<pos_t, Selector, Lenght> S_pos_t; ///< Selector position type (steps)
typedef AxisUnit<steps_t, Selector, Speed> S_speed_t; ///< Selector speed type (steps/s)
typedef AxisUnit<steps_t, Selector, Accel> S_accel_t; ///< Selector acceleration type (steps/s2)

static constexpr S_pos_t operator"" _S_mm(long double mm) {
    return { unitToAxisUnit<S_pos_t>(config::selector.stepsPerUnit, config::U_mm { mm }) };
}

static constexpr S_speed_t operator"" _S_mm_s(long double mm_s) {
    return { unitToAxisUnit<S_speed_t>(config::selector.stepsPerUnit, config::U_mm_s { mm_s }) };
}

static constexpr S_accel_t operator"" _S_mm_s2(long double mm_s2) {
    return { unitToAxisUnit<S_accel_t>(config::selector.stepsPerUnit, config::U_mm_s2 { mm_s2 }) };
}

typedef AxisUnit<pos_t, Idler, Lenght> I_pos_t; ///< Idler position type (steps)
typedef AxisUnit<steps_t, Idler, Speed> I_speed_t; ///< Idler speed type (steps/s)
typedef AxisUnit<steps_t, Idler, Accel> I_accel_t; ///< Idler acceleration type (steps/s2)

static constexpr I_pos_t operator"" _I_deg(long double deg) {
    return { unitToAxisUnit<I_pos_t>(config::idler.stepsPerUnit, config::U_deg { deg }) };
}

static constexpr I_speed_t operator"" _I_deg_s(long double deg_s) {
    return { unitToAxisUnit<I_speed_t>(config::idler.stepsPerUnit, config::U_deg_s { deg_s }) };
}

static constexpr I_accel_t operator"" _I_deg_s2(long double deg_s2) {
    return { unitToAxisUnit<I_accel_t>(config::idler.stepsPerUnit, config::U_deg_s2 { deg_s2 }) };
}

} // namespace motion
} // namespace modules

// Inject literal operators into the global namespace for convenience
using modules::motion::operator"" _P_mm;
using modules::motion::operator"" _P_mm_s;
using modules::motion::operator"" _P_mm_s2;
using modules::motion::operator"" _S_mm;
using modules::motion::operator"" _S_mm_s;
using modules::motion::operator"" _S_mm_s2;
using modules::motion::operator"" _I_deg;
using modules::motion::operator"" _I_deg_s;
using modules::motion::operator"" _I_deg_s2;
