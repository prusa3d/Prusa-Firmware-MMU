#include "hal/cpu.h"
#include "hal/adc.h"
#include "hal/gpio.h"
#include "hal/shr16.h"
#include "hal/spi.h"
#include "hal/usart.h"

#include "pins.h"
#include <avr/interrupt.h>
#include <util/delay.h>

#include "modules/buttons.h"
#include "modules/finda.h"
#include "modules/fsensor.h"
#include "modules/globals.h"
#include "modules/idler.h"
#include "modules/leds.h"
#include "modules/protocol.h"
#include "modules/selector.h"
#include "modules/user_input.h"

#include "logic/command_base.h"
#include "logic/cut_filament.h"
#include "logic/eject_filament.h"
#include "logic/load_filament.h"
#include "logic/no_command.h"
#include "logic/tool_change.h"
#include "logic/unload_filament.h"

namespace mb = modules::buttons;
namespace mp = modules::protocol;
namespace mf = modules::finda;
namespace mfs = modules::fsensor;
namespace mi = modules::idler;
namespace ml = modules::leds;
namespace ms = modules::selector;
namespace mg = modules::globals;
namespace mu = modules::user_input;

namespace hu = hal::usart;

/// Global instance of the protocol codec
static mp::Protocol protocol;

/// A command that resulted in the currently on-going operation
logic::CommandBase *currentCommand = &logic::noCommand;

/// remember the request message that started the currently running command
mp::RequestMsg currentCommandRq(mp::RequestMsgCodes::unknown, 0);

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
    hu::usart1.puts("1234567890\n");
    hu::usart1.puts("1234567890\n");
    hu::usart1.puts("1234567890\n");
    hu::usart1.puts("1234567890\n");
    hu::usart1.puts("1234567890\n");
    hu::usart1.puts("1234567890\n");
    hu::usart1.puts("1234567890\n");
    hu::usart1.puts("1234567890\n");
    hu::usart1.puts("1234567890\n");
    hu::usart1.puts("1234567890\n");
    hu::usart1.puts("1234567890\n");
}

/// One-time setup of HW and SW components
/// Called before entering the loop() function
/// Green LEDs signalize the progress of initialization. If anything goes wrong we shall turn on a red LED
void setup() {
    using namespace hal;

    cpu::Init();

    mg::globals.Init();

    // watchdog init

    shr16::shr16.Init();
    ml::leds.SetMode(4, ml::Color::green, ml::Mode::blink0);
    ml::leds.Step();

    // @@TODO if the shift register doesn't work we really can't signalize anything, only internal variables will be accessible if the UART works

    hu::USART::USART_InitTypeDef usart_conf = {
        .rx_pin = gpio::GPIO_pin(GPIOD, 2),
        .tx_pin = gpio::GPIO_pin(GPIOD, 3),
        .baudrate = 115200,
    };
    hu::usart1.Init(&usart_conf);
    ml::leds.SetMode(3, ml::Color::green, ml::Mode::on);
    ml::leds.Step();

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
    ml::leds.SetMode(2, ml::Color::green, ml::Mode::on);
    ml::leds.Step();

    // tmc::Init()
    ml::leds.SetMode(1, ml::Color::green, ml::Mode::on);
    ml::leds.Step();

    // adc::Init();
    ml::leds.SetMode(0, ml::Color::green, ml::Mode::on);
    ml::leds.Step();
}

void SendMessage(const mp::ResponseMsg &msg) {
}

void PlanCommand(const mp::RequestMsg &rq) {
    if (currentCommand->Error() == ErrorCode::OK) {
        // we are allowed to start a new command as the previous one is in the OK finished state
        switch (rq.code) {
        case mp::RequestMsgCodes::Cut:
            currentCommand = &logic::cutFilament;
            break;
        case mp::RequestMsgCodes::Eject:
            currentCommand = &logic::ejectFilament;
            break;
        case mp::RequestMsgCodes::Load:
            currentCommand = &logic::loadFilament;
            break;
        case mp::RequestMsgCodes::Tool:
            currentCommand = &logic::toolChange;
            break;
        case mp::RequestMsgCodes::Unload:
            currentCommand = &logic::unloadFilament;
            break;
        default:
            currentCommand = &logic::noCommand;
            break;
        }
        currentCommand->Reset(rq.value);
    }
}

void ReportRunningCommand() {
    mp::ResponseMsgParamCodes commandStatus;
    uint8_t value = 0;
    switch (currentCommand->Error()) {
    case ErrorCode::RUNNING:
        commandStatus = mp::ResponseMsgParamCodes::Processing;
        value = (uint8_t)currentCommand->State();
        break;
    case ErrorCode::OK:
        commandStatus = mp::ResponseMsgParamCodes::Finished;
        break;
    default:
        commandStatus = mp::ResponseMsgParamCodes::Error;
        value = (uint8_t)currentCommand->Error();
        break;
    }
    SendMessage(mp::ResponseMsg(currentCommandRq, commandStatus, value));
}

void ProcessRequestMsg(const mp::RequestMsg &rq) {
    switch (rq.code) {
    case mp::RequestMsgCodes::Button:
        // behave just like if the user pressed a button
        mu::userInput.ProcessMessage(rq.value);
        break;
    case mp::RequestMsgCodes::Finda:
        // immediately report FINDA status
        SendMessage(mp::ResponseMsg(rq, mp::ResponseMsgParamCodes::Accepted, mf::finda.Pressed()));
        break;
    case mp::RequestMsgCodes::Mode:
        // immediately switch to normal/stealth as requested
        // modules::motion::SetMode();
        break;
    case mp::RequestMsgCodes::Query:
        // immediately report progress of currently running command
        ReportRunningCommand();
        break;
    case mp::RequestMsgCodes::Reset:
        // immediately reset the board - there is no response in this case
        break; // @@TODO
    case mp::RequestMsgCodes::Version:
        SendMessage(mp::ResponseMsg(rq, mp::ResponseMsgParamCodes::Accepted, 1)); // @@TODO
    case mp::RequestMsgCodes::Wait:
        break; // @@TODO
    case mp::RequestMsgCodes::Cut:
    case mp::RequestMsgCodes::Eject:
    case mp::RequestMsgCodes::Load:
    case mp::RequestMsgCodes::Tool:
    case mp::RequestMsgCodes::Unload:
        PlanCommand(rq);
        break;
        //    case mp::RequestMsgCodes::SetVar: //@@TODO - this is where e.g. printer's fsensor gets updated
        //        SetVar(rq);
        //        break;
    default:
        // respond with an error message
        break;
    }
}

/// @returns true if a request was successfully finished
bool CheckMsgs() {
    using mpd = mp::DecodeStatus;
    while (!hu::usart1.ReadEmpty()) {
        switch (protocol.DecodeRequest(hu::usart1.Read())) {
        case mpd::MessageCompleted:
            // process the input message
            return true;
            break;
        case mpd::NeedMoreData:
            // just continue reading
            break;
        case mpd::Error:
            // @@TODO what shall we do? Start some watchdog? We cannot send anything spontaneously
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
    mb::buttons.Step();
    ml::leds.Step();
    mf::finda.Step();
    mfs::fsensor.Step();
    mi::idler.Step();
    ms::selector.Step();
    mu::userInput.Step();
    currentCommand->Step();
    // add a watchdog reset
}

int main() {
    setup();
    sei(); ///enable interrupts
    for (;;) {
        loop();
    }
    return 0;
}
