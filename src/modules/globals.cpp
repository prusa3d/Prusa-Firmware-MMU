/// @file globals.cpp
#include "globals.h"
#include "permanent_storage.h"

namespace modules {
namespace globals {

Globals globals;

void Globals::Init() {
    mps::FilamentLoaded::get(activeSlot); //@@TODO check for errors
    // @@TODO where to obtain information whether a slot is loaded with a filament?
}

uint8_t Globals::ActiveSlot() const {
    return activeSlot;
}

void Globals::SetActiveSlot(uint8_t newActiveSlot) {
    activeSlot = newActiveSlot;
    mps::FilamentLoaded::set(activeSlot);
}

FilamentLoadState Globals::FilamentLoaded() const {
    return filamentLoaded;
}

void Globals::SetFilamentLoaded(FilamentLoadState newFilamentLoaded) {
    filamentLoaded = newFilamentLoaded;
}

uint16_t Globals::DriveErrors() const {
    return mps::DriveError::get();
}

void Globals::IncDriveErrors() {
    mps::DriveError::increment();
}

void Globals::SetMotorsMode(bool stealth) {
    stealthMode = stealth;
    // @@TODO store into EEPROM
}

} // namespace globals
} // namespace modules
