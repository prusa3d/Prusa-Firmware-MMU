/// @file undervoltage_check.h
#pragma once

#include <stdint.h>
#include "../config/config.h"

/// The modules namespace contains models of MMU's components
namespace modules {
namespace undervoltage_check {

class Undervoltage_check {
public:
    inline constexpr Undervoltage_check() = default;

    /// Reads the ADC, checks the value
    void Step();
};

/// The one and only instance of Undervoltage_check in the FW
extern Undervoltage_check uv_vcc;

} // namespace undervoltage_check
} // namespace modules

namespace muv = modules::undervoltage_check;
