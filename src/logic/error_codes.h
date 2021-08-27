#pragma once
#include <stdint.h>

/// A complete set of error codes which may be a result of a high-level command/operation.
/// This header file shall be included in the printer's firmware as well as a reference,
/// therefore the error codes have been extracted to one place.
///
/// Please note the errors are intentionally coded as "negative" values (highest bit set),
/// becase they are a complement to reporting the state of the high-level state machines -
/// positive values are considered as normal progress, negative values are errors.
///
/// Please note, that multiple TMC errors can occur at once, thus they are defined as a bitmask of the higher byte.
/// Also, as there are 3 TMC drivers on the board, each error is added a bit for the corresponding TMC -
/// TMC_PULLEY_BIT, TMC_SELECTOR_BIT, TMC_IDLER_BIT,
/// The resulting error is a bitwise OR over 3 TMC drivers and their status, which should cover most of the situations correctly.
enum class ErrorCode : uint_fast16_t {
    RUNNING = 0x0000, ///< the operation is still running - keep this value as ZERO as it is used for initialization of error codes as well
    OK = 0x0001, ///< the operation finished OK

    /// Unload Filament related error codes
    FINDA_DIDNT_SWITCH_ON = 0x8001, ///< FINDA didn't switch on while loading filament - either there is something blocking the metal ball or a cable is broken/disconnected
    FINDA_DIDNT_SWITCH_OFF = 0x8002, ///< FINDA didn't switch off while unloading filament

    FSENSOR_DIDNT_SWITCH_ON = 0x8003, ///< Filament sensor didn't switch on while performing LoadFilament
    FSENSOR_DIDNT_SWITCH_OFF = 0x8004, ///< Filament sensor didn't switch off while performing UnloadFilament

    FILAMENT_ALREADY_LOADED = 0x8005, ///< cannot perform operation LoadFilament or move the selector as the filament is already loaded

    INVALID_TOOL = 0x8006, ///< tool/slot index out of range (typically issuing T5 into an MMU with just 5 slots - valid range 0-4)

    MMU_NOT_RESPONDING = 0x802e, ///< internal error of the printer - communication with the MMU is not working

    INTERNAL = 0x802f, ///< internal runtime error (software)

    // TMC bit masks

    TMC_PULLEY_BIT = 0x0040, ///< TMC Pulley bit
    TMC_SELECTOR_BIT = 0x0080, ///< TMC Pulley bit
    TMC_IDLER_BIT = 0x0100, ///< TMC Pulley bit

    TMC_IOIN_MISMATCH = 0x8200, ///< TMC driver init error - TMC dead or bad communication

    /// TMC driver reset - recoverable, we just need to rehome the axis
    /// Idler: can be rehomed any time
    /// Selector: if there is a filament, remove it and rehome, if there is no filament, just rehome
    /// Pulley: do nothing - for the loading sequence - just restart and move slowly, for the unload sequence just restart
    TMC_RESET = 0x8400,

    TMC_UNDERVOLTAGE_ON_CHARGE_PUMP = 0x8800, ///< not enough current for the TMC, NOT RECOVERABLE

    TMC_SHORT_TO_GROUND = 0x9000, ///< TMC driver serious error - short to ground on coil A or coil B - dangerous to recover

    /// TMC driver over temperature warning - can be recovered by restarting the driver.
    /// If this error happens, we should probably go into the error state as soon as the current command is finished.
    /// The driver technically still works at this point.
    TMC_OVER_TEMPERATURE_WARN = 0xA000,

    /// TMC driver over temperature error - we really shouldn't ever reach this error.
    /// It can still be recovered if the driver cools down below 120C.
    /// The driver needs to be disabled and enabled again for operation to resume after this error is cleared.
    TMC_OVER_TEMPERATURE_ERROR = 0xC000
};
