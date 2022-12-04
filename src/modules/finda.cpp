/// @file finda.cpp
#include "finda.h"
#include "globals.h"
#include "timebase.h"
#include "../hal/gpio.h"
#include "../pins.h"

namespace modules {
namespace finda {

FINDA finda;

void FINDA::Step() {
    debounce::Debouncer::Step(mt::timebase.Millis(), hal::gpio::ReadPin(FINDA_PIN) == hal::gpio::Level::high);
}

void FINDA::BlockingInit() {
    uint16_t start = mt::timebase.Millis();
    // let FINDA settle down - we're gonna need its state for selector homing
    while (!mt::timebase.Elapsed(start, config::findaDebounceMs + 1)) {
        Step();
    }
}

bool FINDA::CheckFINDAvsEEPROM() {
    bool ret = true;
    if (mf::finda.Pressed() && mg::globals.FilamentLoaded() < mg::InFSensor) {
        // This is a tricky situation - EEPROM doesn't have a record about loaded filament (blocking the Selector)
        // To be on the safe side, we have to override the EEPROM information about filament position - at least InFSensor
        // Moreover - we need to override the active slot position as well, because we cannot know where the Selector is.
        // For this we speculatively set the active slot to 2 (in the middle ;) )
        // Ideally this should be signalled as an error state and displayed on the printer and recovered properly.
        //mg::globals.SetFilamentLoaded(2, mg::InFSensor);
        ret = false;
    } else if (!mf::finda.Pressed() && mg::globals.FilamentLoaded() >= mg::InSelector) {
        // Opposite situation - not so dangerous, but definitely confusing to users.
        // FINDA is not pressed but we have a record in the EEPROM.
        // It has been decided, that we shall clear such a record from EEPROM automagically
        // and presume there is no filament at all (requires working FINDA)
        //mg::globals.SetFilamentLoaded(config::toolCount, mg::AtPulley);
        ret = false;
    }
    return ret;
}

} // namespace finda
} // namespace modules
