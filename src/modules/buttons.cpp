#include "buttons.h"

namespace modules {
namespace buttons {

Buttons buttons;

uint16_t Buttons::tmpTiming = 0;

int8_t Buttons::Sample(uint16_t rawADC) {
    // decode 3 buttons' levels from one ADC
    // Button 1 - 0
    // Button 2 - 344
    // Button 3 - 516
    // Doesn't handle multiple pressed buttons at once

    if (rawADC < 10)
        return 0;
    else if (rawADC > 320 && rawADC < 360)
        return 1;
    else if (rawADC > 500 && rawADC < 530)
        return 2;
    return -1;
}

void Buttons::Step(uint16_t rawADC) {
    // @@TODO temporary timing
    ++tmpTiming;
    int8_t currentState = Sample(rawADC);
    for (uint_fast8_t b = 0; b < N; ++b) {
        // this button was pressed if b == currentState, released otherwise
        buttons[b].Step(tmpTiming, b == currentState);
    }
}

} // namespace buttons
} // namespace modules
