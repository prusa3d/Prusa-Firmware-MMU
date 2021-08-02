#include "main_loop_stub.h"

#include "../../modules/stubs/stub_adc.h"
#include "../../modules/stubs/stub_timebase.h"

#include "../../../../src/modules/buttons.h"
#include "../../../../src/modules/finda.h"
#include "../../../../src/modules/fsensor.h"
#include "../../../../src/modules/globals.h"
#include "../../../../src/modules/idler.h"
#include "../../../../src/modules/leds.h"
#include "../../../../src/modules/motion.h"
#include "../../../../src/modules/permanent_storage.h"
#include "../../../../src/modules/selector.h"
#include "../../../../src/modules/user_input.h"

#include "../stubs/stub_motion.h"

#include <new> // bring in placement new
#include <stddef.h>

void main_loop() {
    mb::buttons.Step();
    ml::leds.Step();
    mf::finda.Step();
    mfs::fsensor.Step();
    mi::idler.Step();
    ms::selector.Step();
    mm::motion.Step();
    mui::userInput.Step();

    mt::IncMillis();
}

void ForceReinitAllAutomata() {
    // This woodoo magic with placement new is just a forced reinit of global instances of firmware's state machines
    // just for the purposes of separate unit tests. Each unit test needs a "freshly booted firmware" and since all unit tests
    // in the test binary share the same global data structures, we need some way of making them fresh each time.
    //
    // This approach mimics the firmware behavior closely as the firmware initializes its global data structures
    // on its very start once (by copying static init data from PROGMEM into RAM) - and we need exactly this approach in the unit tests.
    //
    // There are multiple other approaches, one of them is adding a special Init() function into each of these state machines.
    // As this approach might look like a standard and safer way of doing stuff, it has several drawbacks, especially
    // it needs an explicit call to the Init function every time an object like this is created - this can have negative influence on firmware's code size

    new (&mb::buttons) mb::Buttons();
    new (&ml::leds) ml::LEDs();
    new (&mf::finda) mf::FINDA();
    new (&mfs::fsensor) mfs::FSensor();
    new (&mi::idler) mi::Idler();
    new (&ms::selector) ms::Selector();
    new (&mm::motion) mm::Motion();

    // no buttons involved ;)
    hal::adc::ReinitADC(config::buttonsADCIndex, hal::adc::TADCData({ 1023 }), 1);

    // finda OFF
    hal::adc::ReinitADC(config::findaADCIndex, hal::adc::TADCData({ 0 }), 1);

    // reinit timing
    mt::ReinitTimebase();

    // reinit axes positions
    mm::ReinitMotion();

    // let's assume we have the filament NOT loaded and active slot 0
    mg::globals.SetFilamentLoaded(false);
    mg::globals.SetActiveSlot(0);
}

void EnsureActiveSlotIndex(uint8_t slot) {
    // move selector to the right spot
    ms::selector.MoveToSlot(slot);
    while (ms::selector.Slot() != slot)
        main_loop();

    mg::globals.SetActiveSlot(slot);
}

void SetFINDAStateAndDebounce(bool press) {
    hal::adc::SetADC(config::findaADCIndex, press ? config::findaADCDecisionLevel + 1 : config::findaADCDecisionLevel - 1);
    for (size_t i = 0; i < config::findaDebounceMs + 1; ++i)
        main_loop();
}
