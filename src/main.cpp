#include "logic/mm_control.h"
#include "hal/gpio.h"
#include "hal/spi.h"
#include "hal/usart.h"
#include "pins.h"
#include <avr/interrupt.h>

/// One-time setup of HW and SW components
/// Called before entering the loop() function
void setup() {
    using namespace hal;

    // spi::SPI_InitTypeDef spi_conf = {
    //     .miso_pin = gpio::GPIO_pin(TMC2130_SPI_MISO_PIN),
    //     .mosi_pin = gpio::GPIO_pin(TMC2130_SPI_MOSI_PIN),
    //     .sck_pin = gpio::GPIO_pin(TMC2130_SPI_SCK_PIN),
    //     .ss_pin = gpio::GPIO_pin(TMC2130_SPI_SS_PIN),
    //     .prescaler = 2, //4mhz
    //     .cpha = 1,
    //     .cpol = 1,
    // };
    // spi::Init(SPI0, &spi_conf);

    // // SPI example
    // gpio::Init(gpio::GPIO_pin(GPIOC, 6), gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::high));
    // uint8_t dat[5];
    // gpio::WritePin(gpio::GPIO_pin(GPIOC, 6), gpio::Level::low);
    // spi::TxRx(SPI0, 0x01);
    // spi::TxRx(SPI0, 0x00);
    // spi::TxRx(SPI0, 0x00);
    // spi::TxRx(SPI0, 0x00);
    // spi::TxRx(SPI0, 0x00);
    // gpio::WritePin(gpio::GPIO_pin(GPIOC, 6), gpio::Level::high);
    // gpio::WritePin(gpio::GPIO_pin(GPIOC, 6), gpio::Level::low);
    // dat[0] = spi::TxRx(SPI0, 0x00);
    // dat[1] = spi::TxRx(SPI0, 0x00);
    // dat[2] = spi::TxRx(SPI0, 0x00);
    // dat[3] = spi::TxRx(SPI0, 0x00);
    // dat[4] = spi::TxRx(SPI0, 0x00);
    // gpio::WritePin(gpio::GPIO_pin(GPIOC, 6), gpio::Level::high);
    // (void)dat;

    USART::USART_InitTypeDef usart_conf = {
        .rx_pin = gpio::GPIO_pin(GPIOD, 2),
        .tx_pin = gpio::GPIO_pin(GPIOD, 3),
        .baudrate = 115200,
    };

    usart1.Init(&usart_conf);
    sei();
    usart1.puts("1234567890\n");
    usart1.puts("1234567890\n");
    usart1.puts("1234567890\n");
    usart1.puts("1234567890\n");
    usart1.puts("1234567890\n");
    usart1.puts("1234567890\n");
    usart1.puts("1234567890\n");
    usart1.puts("1234567890\n");
    usart1.puts("1234567890\n");
    usart1.puts("1234567890\n");
    usart1.puts("1234567890\n");
    // usart1.Flush();
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
        if (!usart1.ReadEmpty())
            usart1.Write(usart1.Read());
        loop();
    }
    return 0;
}
