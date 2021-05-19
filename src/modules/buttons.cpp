#include "buttons.h"
#include "../hal/adc.h"

namespace modules {

uint16_t Buttons::tmpTiming = 0;

// original idea from: https://www.eeweb.com/debouncing-push-buttons-using-a-state-machine-approach
void Button::Step(uint16_t time, bool press) {
    switch (f.state) {
    case State::Waiting:
        if (press) {
            f.state = State::Detected;
            timeLastChange = time;
            f.tmp = press;
        }
        break;
    case State::Detected:
        if (f.tmp == press) {
            if (time - timeLastChange > debounce) {
                f.state = State::WaitForRelease;
            }
        } else {
            f.state = State::Waiting;
        }
        break;
    case State::WaitForRelease:
        if (!press) {
            f.state = State::Update;
        }
        break;
    case State::Update:
        f.state = State::Waiting;
        timeLastChange = time;
        f.tmp = false;
        break;
    default:
        f.state = State::Waiting;
        timeLastChange = time;
        f.tmp = false;
    }
}

int8_t Buttons::Sample() {
    // decode 3 buttons' levels from one ADC
    uint16_t raw = hal::ADC::ReadADC(0);

    // Button 1 - 0
    // Button 2 - 344
    // Button 3 - 516
    // Doesn't handle multiple pressed buttons at once

    if (raw < 10)
        return 0;
    else if (raw > 320 && raw < 360)
        return 1;
    else if (raw > 500 && raw < 530)
        return 2;
    return -1;
}

void Buttons::Step() {
    // @@TODO temporary timing
    ++tmpTiming;
    int8_t currentState = Sample();
    for (uint_fast8_t b = 0; b < N; ++b) {
        // this button was pressed if b == currentState, released otherwise
        buttons[b].Step(tmpTiming, b == currentState);
    }
}

} // namespace modules
