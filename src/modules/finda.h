#pragma once
#include <stdint.h>
#include "debouncer.h"

namespace modules {
namespace finda {

class FINDA : protected debounce::Debouncer {
public:
    inline constexpr FINDA()
        : debounce::Debouncer(debounce) {};
    void Step();
    using debounce::Debouncer::Pressed;

private:
    /// time interval for debouncing @@TODO specify units
    constexpr static const uint16_t debounce = 100;
    /// ADC decision level when a FINDA is considered pressed/not pressed
    constexpr static const uint16_t adcDecisionLevel = 512;
};

extern FINDA finda;

} // namespace finda
} // namespace modules
