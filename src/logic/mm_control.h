#pragma once
#include <stdint.h>

/// @@TODO @3d-gussner
/// Extract the current state machines of high-level operations (load fillament, unload fillament etc.) here
/// Design some nice non-blocking API for these operations
///
/// Which automatons are high-level? Those which are being initiated either by a command over the serial line or from a button
/// - they report their progress to the printer
/// - they can be composed of other sub automatons

namespace logic {

/// Tasks derived from this base class are the top-level operations inhibited by the printer.
/// These tasks report their progress and only one of these tasks is allowed to run at once.
class TaskBase {
public:
    inline TaskBase() = default;

    virtual void Reset() = 0;
    virtual bool Step() = 0;
    /// probably individual states of the automaton
    virtual uint8_t Progress() const = 0;
    /// @@TODO cleanup status codes
    /// @returns 0 if the operation is still running
    ///          1 if the operation finished OK
    ///          >=2 if the operation failed - the value is the error code
    virtual int8_t Status() const = 0;

protected:
    uint8_t state;
};

class Logic {

public:
    inline Logic() = default;

    void UnloadFilament();
};

} // namespace logic
