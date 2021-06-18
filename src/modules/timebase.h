#pragma once
#include <stdint.h>

namespace modules {
namespace time {

/// A basic time tracking class
/// Works on top of processor timers and provides real-time steady clock
/// (at least what the CPU thinks ;) )
class Timebase {
public:
    constexpr inline Timebase()
        : ms(0) {}
    void Init();

    /// @returns current milliseconds elapsed from the initialization of this class
    ///  (usually the start of the firmware)
    uint16_t Millis() const;

private:
    uint16_t ms;
    static void ISR();
};

extern Timebase timebase;

} // namespace time
} // namespace modules
