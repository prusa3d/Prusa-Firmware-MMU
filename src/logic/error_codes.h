#pragma once
#include <stdint.h>

/// A complete set of error codes which may be a result of a high-level command/operation
/// This header file shall be included in the printer's firmware as well as a reference,
/// therefore the error codes have been extracted to one place
enum class ErrorCode : int_fast8_t {
    RUNNING = 0, ///< the operation is still running
    OK, ///< the operation finished OK

    /// Unload Filament related error codes
    FINDA_DIDNT_SWITCH_ON = -1, ///< FINDA didn't switch on while loading filament - either there is something blocking the metal ball or a cable is broken/disconnected
    FINDA_DIDNT_SWITCH_OFF = -2, ///< FINDA didn't switch off while unloading filament

    FSENSOR_DIDNT_SWITCH_ON = -3, ///< Filament sensor didn't switch on while performing LoadFilament
    FSENSOR_DIDNT_SWITCH_OFF = -4, ///< Filament sensor didn't switch off while performing UnloadFilament

    FILAMENT_ALREADY_LOADED = -5, ///< cannot perform operation LoadFilament or move the selector as the filament is already loaded

    TMC_INIT_ERROR = -6, ///< TMC driver init error - the MMU cannot move one motor due to a HW problem

    MMU_NOT_RESPONDING = -126, ///< internal error of the printer - communication with the MMU is not working

    INTERNAL = -127, ///< internal runtime error (software)
};
