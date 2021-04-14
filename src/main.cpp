#include "logic/mm_control.h"

/// One-time setup of HW and SW components
/// Called before entering the loop() function
void setup(){

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
        loop();
    }
    return 0;
}
