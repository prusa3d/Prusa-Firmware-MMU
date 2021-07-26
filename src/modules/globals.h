#pragma once
#include <stdint.h>

namespace modules {

/// The globals namespace provides all necessary facilities related to keeping track of global state of the firmware.
namespace globals {

/// Globals keep track of global state variables in the firmware.
/// So far only Active slot and Filament loaded variables are used.
class Globals {
public:
    /// Initializes the global storage hive - basically looks into EEPROM to gather information.
    void Init();

    /// @returns active filament slot on the MMU unit
    /// Slots are numbered 0-4
    uint8_t ActiveSlot() const;

    /// Sets the active slot, usually after some command/operation.
    /// Also updates the EEPROM records accordingly
    /// @param newActiveSlot the new slot index to set
    void SetActiveSlot(uint8_t newActiveSlot);

    /// @returns true if filament is considered as loaded
    ///  @@TODO this may change meaning slightly as the MMU is primarily concerned
    ///  about whether a piece of filament is stock up of a PTFE tube.
    ///  If that's true, we cannot move the selector.
    bool FilamentLoaded() const;

    /// Sets the filament loaded flag value, usually after some command/operation.
    /// Also updates the EEPROM records accordingly
    /// @param newFilamentLoaded new state
    void SetFilamentLoaded(bool newFilamentLoaded);

    /// @returns the total number of MMU errors so far
    /// Errors are stored in the EEPROM
    uint16_t DriveErrors() const;

    /// Increment MMU errors by 1
    void IncDriveErrors();

private:
    uint8_t activeSlot;
    bool filamentLoaded;
};

/// The one and only instance of global state variables
extern Globals globals;

} // namespace globals
} // namespace modules
