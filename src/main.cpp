#include "logic/mm_control.h"
#include "hal/gpio.h"
#include "hal/spi.h"

/// One-time setup of HW and SW components
/// Called before entering the loop() function
void setup() {
    using namespace hal;

    spi::SPI_InitTypeDef spi_conf = {
        .miso_pin = gpio::GPIO_pin(GPIOB, 3),
        .mosi_pin = gpio::GPIO_pin(GPIOB, 2),
        .sck_pin = gpio::GPIO_pin(GPIOB, 1),
        .ss_pin = gpio::GPIO_pin(GPIOB, 0),
        .prescaler = 2, //4mhz
    };
    spi::Init(SPI0, &spi_conf);
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
void loop() {
}

int main() {
    setup();
    for (;;) {
        using namespace hal::gpio;
        WritePin(GPIO_pin(GPIOB, 5), Level::low);
        TogglePin(GPIO_pin(GPIOB, 6));
        if (hal::gpio::ReadPin(GPIO_pin(GPIOB, 7)) == hal::gpio::Level::low)
            break;
        loop();
    }
    return 0;
}
