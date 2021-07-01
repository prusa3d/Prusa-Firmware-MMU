#include "buttons.h"
#include "../hal/adc.h"
#include "timebase.h"

namespace modules {
namespace buttons {

Buttons buttons;

int8_t Buttons::DecodeADC(uint16_t rawADC) {
    // decode 3 buttons' levels from one ADC
    // Button 1 - 0
    // Button 2 - 344
    // Button 3 - 516
    // Doesn't handle multiple pressed buttons at once

    if (rawADC >= config::button0ADCMin && rawADC <= config::button0ADCMax)
        return 0;
    else if (rawADC >= config::button1ADCMin && rawADC <= config::button1ADCMax)
        return 1;
    else if (rawADC >= config::button2ADCMin && rawADC <= config::button2ADCMax)
        return 2;
    return -1;
}

void Buttons::Step() {
    uint16_t millis = modules::time::timebase.Millis();
    int8_t currentState = DecodeADC(hal::adc::ReadADC(config::buttonsADCIndex));
    for (uint_fast8_t b = 0; b < N; ++b) {
        // this button was pressed if b == currentState, released otherwise
        buttons[b].Step(millis, b == currentState);
    }
}

} // namespace buttons
} // namespace modules
