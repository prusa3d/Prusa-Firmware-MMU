#include "main_loop_stub.h"
#include "../../../../src/modules/buttons.h"
#include "../../../../src/modules/finda.h"
#include "../../../../src/modules/fsensor.h"
#include "../../../../src/modules/globals.h"
#include "../../../../src/modules/idler.h"
#include "../../../../src/modules/leds.h"
#include "../../../../src/modules/permanent_storage.h"
#include "../../../../src/modules/selector.h"

logic::CommandBase *currentCommand = nullptr;

// just like in the real FW, step all the known automata
void main_loop() {
    modules::buttons::buttons.Step(hal::adc::ReadADC(0));
    modules::leds::leds.Step(0);
    modules::finda::finda.Step(0);
    modules::fsensor::fsensor.Step(0);
    modules::idler::idler.Step();
    modules::selector::selector.Step();
    currentCommand->Step();
}
