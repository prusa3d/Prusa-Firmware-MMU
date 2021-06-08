#pragma once
#include <stdint.h>

/// A complete set of error codes which may be a result of a high-level command/operation
/// This header file shall be included in the printer's firmware as well as a reference,
/// therefore the error codes have been extracted to one place

enum class ErrorCode : int_fast8_t {
    RUNNING = 0, ///< the operation is still running
    OK, ///< the operation finished OK

    /// Unload Filament related error codes
    UNLOAD_FINDA_DIDNT_TRIGGER = -1, ///< FINDA didn't trigger while unloading filament - either there is something blocking the metal ball or a cable is broken/disconnected
    UNLOAD_ERROR2 = -2,
};
