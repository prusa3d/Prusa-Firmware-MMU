#include "main_loop_stub.h"
#include "../../../../src/modules/buttons.h"
#include "../../../../src/modules/finda.h"
#include "../../../../src/modules/fsensor.h"
#include "../../../../src/modules/globals.h"
#include "../../../../src/modules/idler.h"
#include "../../../../src/modules/leds.h"
#include "../../../../src/modules/motion.h"
#include "../../../../src/modules/permanent_storage.h"
#include "../../../../src/modules/selector.h"

logic::CommandBase *currentCommand = nullptr;

// just like in the real FW, step all the known automata
uint16_t tmpTiming = 0;

void main_loop() {
    modules::buttons::buttons.Step(hal::adc::ReadADC(0));
    modules::leds::leds.Step(tmpTiming);
    modules::finda::finda.Step(tmpTiming);
    modules::fsensor::fsensor.Step(tmpTiming);
    modules::idler::idler.Step();
    modules::selector::selector.Step();
    modules::motion::motion.Step();
    if (currentCommand)
        currentCommand->Step();
    ++tmpTiming;
}
