#pragma once
#include <stdint.h>
#include "error_codes.h"
#include "progress_codes.h"

/// The logic namespace handles the application logic on top of the modules.
namespace logic {

/// @brief Base class defining common API for high-level operations/commands/state machines
///
/// Which state machines are high-level? Those which are being initiated either by a command over the serial line or from a button
/// - they report their progress to the printer
/// - they can be composed of other sub automata
///
/// Tasks derived from this base class are the top-level operations inhibited by the printer.
/// These tasks report their progress and only one of these tasks is allowed to run at once.
class CommandBase {
public:
    inline CommandBase()
        : state(ProgressCode::OK)
        , error(ErrorCode::OK) {}

    // Normally, a base class should (must) have a virtual destructor to enable correct deallocation of superstructures.
    // However, in our case we don't want ANY destruction of these objects and moreover - adding a destructor like this
    // makes the linker complain about missing operator delete(), which is really not something we want/need in our case.
    // Without the destructor, the linker is "happy" ;)
    // virtual ~CommandBase() = default;

    /// resets the automaton
    /// @param param numerical parameter that comes with some commands (e.g. T1 for tool change 1)
    virtual void Reset(uint8_t param) = 0;

    /// steps the state machine
    /// @returns true if the automaton finished its work
    virtual bool Step() = 0;

    /// @returns progress of operation - each automaton consists of several internal states
    /// which should be reported to the user via the printer's LCD
    /// E.g. Tool change: first tries to unload filament, then selects another slot and then tries to load filament
    ///
    /// Beware - derived automata report detailed states of underlying state machines if any
    /// E.g. Eject filament first tries to unload filament, which is a standalone automaton.
    /// Therefore until the unload is finished, this method will report the internal state of Unload filament.
    /// The reason for this is to be able to report exactly what is happening to the printer, especially loading and unloading sequences (and errors)
    virtual ProgressCode State() const { return state; }

    /// @returns progress of operation of only this state machine - regardless of any underlying automata (if any)
    /// Therefore it is not a vitual method.
    ProgressCode TopLevelState() const { return state; }

    /// @returns status of the operation - e.g. RUNNING, OK, or an error code if the operation failed.
    ///
    /// Beware - the same rule about composite operations as with State() applies to Error() as well.
    /// Please see ErrorCode for more details
    virtual ErrorCode Error() const { return error; }

protected:
    ProgressCode state; ///< current progress state of the state machine
    ErrorCode error; ///< current error code
};

} // namespace logic
