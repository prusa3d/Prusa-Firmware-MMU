#include "hal/cpu.h"
#include "hal/adc.h"
#include "hal/gpio.h"
#include "hal/spi.h"
#include "hal/usart.h"
#include "hal/shr16.h"

#include "pins.h"
#include <avr/interrupt.h>

#include "modules/buttons.h"
#include "modules/leds.h"
#include "modules/protocol.h"

#include "logic/mm_control.h"

static modules::protocol::Protocol protocol;
static modules::buttons::Buttons buttons;
static modules::leds::LEDs leds;

// examples and test code shall be located here
void TmpPlayground() {
    using namespace hal;

    // SPI example
    //    gpio::Init(gpio::GPIO_pin(GPIOC, 6), gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::high));
    //    uint8_t dat[5];
    //    gpio::WritePin(gpio::GPIO_pin(GPIOC, 6), gpio::Level::low);
    //    spi::TxRx(SPI0, 0x01);
    //    spi::TxRx(SPI0, 0x00);
    //    spi::TxRx(SPI0, 0x00);
    //    spi::TxRx(SPI0, 0x00);
    //    spi::TxRx(SPI0, 0x00);
    //    gpio::WritePin(gpio::GPIO_pin(GPIOC, 6), gpio::Level::high);
    //    gpio::WritePin(gpio::GPIO_pin(GPIOC, 6), gpio::Level::low);
    //    dat[0] = spi::TxRx(SPI0, 0x00);
    //    dat[1] = spi::TxRx(SPI0, 0x00);
    //    dat[2] = spi::TxRx(SPI0, 0x00);
    //    dat[3] = spi::TxRx(SPI0, 0x00);
    //    dat[4] = spi::TxRx(SPI0, 0x00);
    //    gpio::WritePin(gpio::GPIO_pin(GPIOC, 6), gpio::Level::high);
    //    (void)dat;

    //    using namespace hal::gpio;
    //    WritePin(GPIO_pin(GPIOB, 5), Level::low);
    //    TogglePin(GPIO_pin(GPIOB, 6));
    //    if (hal::gpio::ReadPin(GPIO_pin(GPIOB, 7)) == hal::gpio::Level::low)
    //        break;

    sei();
    hal::usart::usart1.puts("1234567890\n");
    hal::usart::usart1.puts("1234567890\n");
    hal::usart::usart1.puts("1234567890\n");
    hal::usart::usart1.puts("1234567890\n");
    hal::usart::usart1.puts("1234567890\n");
    hal::usart::usart1.puts("1234567890\n");
    hal::usart::usart1.puts("1234567890\n");
    hal::usart::usart1.puts("1234567890\n");
    hal::usart::usart1.puts("1234567890\n");
    hal::usart::usart1.puts("1234567890\n");
    hal::usart::usart1.puts("1234567890\n");
}

/// One-time setup of HW and SW components
/// Called before entering the loop() function
/// Green LEDs signalize the progress of initialization. If anything goes wrong we shall turn on a red LED
void setup() {
    using namespace hal;

    cpu::Init();

    shr16::shr16.Init();
    leds.SetMode(4, false, modules::leds::Mode::blink0);
    leds.Step(0);

    // @@TODO if the shift register doesn't work we really can't signalize anything, only internal variables will be accessible if the UART works

    hal::usart::USART::USART_InitTypeDef usart_conf = {
        .rx_pin = gpio::GPIO_pin(GPIOD, 2),
        .tx_pin = gpio::GPIO_pin(GPIOD, 3),
        .baudrate = 115200,
    };
    hal::usart::usart1.Init(&usart_conf);
    leds.SetMode(3, false, modules::leds::Mode::on);
    leds.Step(0);

    // @@TODO if both shift register and the UART are dead, we are sitting ducks :(

    spi::SPI_InitTypeDef spi_conf = {
        .miso_pin = gpio::GPIO_pin(TMC2130_SPI_MISO_PIN),
        .mosi_pin = gpio::GPIO_pin(TMC2130_SPI_MOSI_PIN),
        .sck_pin = gpio::GPIO_pin(TMC2130_SPI_SCK_PIN),
        .ss_pin = gpio::GPIO_pin(TMC2130_SPI_SS_PIN),
        .prescaler = 2, //4mhz
        .cpha = 1,
        .cpol = 1,
    };
    spi::Init(SPI0, &spi_conf);
    leds.SetMode(2, false, modules::leds::Mode::on);
    leds.Step(0);

    // tmc::Init()
    leds.SetMode(1, false, modules::leds::Mode::on);
    leds.Step(0);

    // adc::Init();
    leds.SetMode(0, false, modules::leds::Mode::on);
    leds.Step(0);
}

void ProcessRequestMsg(const modules::protocol::RequestMsg &rq) {
}

/// @returns true if a request was successfully finished
bool CheckMsgs() {
    using mpd = modules::protocol::DecodeStatus;
    while (!hal::usart::usart1.ReadEmpty()) {
        switch (protocol.DecodeRequest(hal::usart::usart1.Read())) {
        case mpd::MessageCompleted:
            // process the input message
            return true;
            break;
        case mpd::NeedMoreData:
            // just continue reading
            break;
        case mpd::Error:
            // what shall we do? Start some watchdog?
            break;
        }
    }
    return false;
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
    if (CheckMsgs()) {
        ProcessRequestMsg(protocol.GetRequestMsg());
    }
    buttons.Step(hal::adc::ReadADC(0));
    leds.Step(0);
}

int main() {
    setup();
    for (;;) {
        loop();
    }
    return 0;
}
