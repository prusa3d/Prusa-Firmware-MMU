/// @file
#pragma once

#include <stdint.h>
#include "../config/config.h"

/// The modules namespace contains models of MMU's components
namespace modules {
namespace voltage {

/// We are measuring the bandgap voltage, Vb=1.1V
/// To compute the threshold value: `VAL = 1125.3 / AVCC`
/// So for:
/// - AVCC=5V, you get VAL=225.06
/// - AVCC=4.1V, you get VAL=274.46
/// - AVCC=4V, you get VAL=281.35
/// - any lower and the board will probably die sooner than being able to report anything
class VCC {
public:
    inline constexpr VCC()
        : vcc_val(0) {}

    /// Reads the ADC, checks the value
    void Step();

    /// @returns the current VCC voltage level, platform dependent.
    /// @note see VCC measurement setup in config.h
    uint16_t CurrentVCC() const { return vcc_val; }

private:
    uint16_t vcc_val;
};

/// The one and only instance of Undervoltage_check in the FW
extern VCC vcc;

} // namespace voltage
} // namespace modules

namespace mv = modules::voltage;
