#pragma once
#include <stdint.h>

namespace modules {

/// The time namespace provides all necessary facilities related to measuring real elapsed time for the whole firmware.
namespace time {

/// A basic time tracking class
/// Works on top of processor timers and provides real-time steady clock
/// (at least what the CPU thinks ;) )
class Timebase {
public:
    constexpr inline Timebase()
        : ms(0) {}

    /// Initializes the Timebase class - sets the timers and prepares the internal variables.
    void Init();

    /// @returns current milliseconds elapsed from the initialization of this class
    ///  (usually the start of the firmware)
    uint16_t Millis() const;

private:
    uint16_t ms;
    static void ISR();
};

/// The one and only instance of Selector in the FW
extern Timebase timebase;

} // namespace time
} // namespace modules
