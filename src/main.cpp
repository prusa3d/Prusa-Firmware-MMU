#include "logic/mm_control.h"
#include "hal/gpio.h"

/// One-time setup of HW and SW components
/// Called before entering the loop() function
void setup(){
    using namespace hal::gpio;
    hal::gpio::Init(GPIOB, 1, GPIO_InitTypeDef(Mode::output, Level::low));

}

/// Main loop of the firmware
/// Proposed architecture
///   checkMsgs();
///   if(msg is command){
///     activate command handling
///   } else if(msg is query){
///     format response to query
///   }
///   StepCurrentCommand();
///   StepMotors();
///   StepLED();
///   StepWhateverElseNeedsStepping();
/// The idea behind the Step* routines is to keep each automaton non-blocking allowing for some “concurrency”.
/// Some FW components will leverage ISR to do their stuff (UART, motor stepping?, etc.)
void loop(){

}

int main() {
    setup();
    for(;;){
        using namespace hal::gpio;
        WritePin(GPIOB, 5, Level::low);
        TogglePin(GPIOB, 6);
        if (hal::gpio::ReadPin(GPIOB, 7) == hal::gpio::Level::low)
            break;
        loop();
    }
    return 0;
}
