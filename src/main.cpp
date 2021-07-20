#include "hal/cpu.h"
#include "hal/adc.h"
#include "hal/gpio.h"
#include "hal/shr16.h"
#include "hal/spi.h"
#include "hal/usart.h"
#include "hal/watchdog.h"

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
#include "modules/timebase.h"
#include "modules/motion.h"

#include "logic/command_base.h"
#include "logic/cut_filament.h"
#include "logic/eject_filament.h"
#include "logic/load_filament.h"
#include "logic/no_command.h"
#include "logic/set_mode.h"
#include "logic/tool_change.h"
#include "logic/unload_filament.h"

#include "version.h"

#include "panic.h"

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

    mt::timebase.Init();
    hal::watchdog::ConfigureWatchDog(8);

    mg::globals.Init();

    // watchdog init

    shr16::shr16.Init();
    ml::leds.SetMode(4, ml::Color::green, ml::Mode::on);
    ml::leds.Step();

    // @@TODO if the shift register doesn't work we really can't signalize anything, only internal variables will be accessible if the UART works

    hu::USART::USART_InitTypeDef usart_conf = {
        .rx_pin = USART_RX,
        .tx_pin = USART_TX,
        .baudrate = 115200,
    };
    hu::usart1.Init(&usart_conf);
    ml::leds.SetMode(3, ml::Color::green, ml::Mode::on);
    ml::leds.Step();

    // @@TODO if both shift register and the UART are dead, we are sitting ducks :(

    spi::SPI_InitTypeDef spi_conf = {
        .miso_pin = TMC2130_SPI_MISO_PIN,
        .mosi_pin = TMC2130_SPI_MOSI_PIN,
        .sck_pin = TMC2130_SPI_SCK_PIN,
        .ss_pin = TMC2130_SPI_SS_PIN,
        .prescaler = 2, //4mhz
        .cpha = 1,
        .cpol = 1,
    };
    spi::Init(SPI0, &spi_conf);
    ml::leds.SetMode(2, ml::Color::green, ml::Mode::on);
    ml::leds.Step();

    mm::Init();
    ml::leds.SetMode(1, ml::Color::green, ml::Mode::on);
    ml::leds.Step();

    adc::Init();
    ml::leds.SetMode(0, ml::Color::green, ml::Mode::on);
    ml::leds.Step();

    /// Turn off all leds
    for (uint8_t i = 0; i < config::toolCount; i++) {
        ml::leds.SetMode(i, ml::Color::green, ml::Mode::off);
        ml::leds.SetMode(i, ml::Color::red, ml::Mode::off);
    }
    ml::leds.Step();
}

static constexpr const uint8_t maxMsgLen = 10;

bool WriteToUSART(const uint8_t *src, uint8_t len) {
    // How to properly enqueue the message? Especially in case of a full buffer.
    // We neither can stay here in an endless loop until the buffer drains.
    // Nor can we save the message elsewhere ... it must be just skipped and the protocol must handle it.
    // Under normal circumstances, such a situation should not happen.
    // The MMU cannot produce response messages on its own - it only responds to requests from the printer.
    // That means there is only one message in the output buffer at once as long as the printer waits for the response before sending another request.
    for (uint8_t i = 0; i < len; ++i) {
        if (hu::usart1.CanWrite()) {
            // should not block waiting for the TX buffer to drain as there was an empty spot for at least 1 byte
            hu::usart1.Write(src[i]);
        } else {
            //buffer full - must skip the rest of the message - the communication will drop out anyway
            return false;
        }
    }
    return true; // not sure if we can actually leverage the knowledge of success while sending the message
}

void ReportCommandAccepted(const mp::RequestMsg &rq, mp::ResponseMsgParamCodes status) {
    uint8_t tmp[maxMsgLen];
    uint8_t len = protocol.EncodeResponseCmdAR(rq, status, tmp);
    WriteToUSART(tmp, len);
}

void ReportFINDA(const mp::RequestMsg &rq) {
#ifdef DEBUG_FINDA
    using namespace hal;
    hu::usart1.puts("FINDA:");
    if (hal::gpio::ReadPin(FINDA_PIN) == hal::gpio::Level::high) {
        hu::usart1.puts(" TIRGGERED\n");
    } else {
        hu::usart1.puts(" NOT TRIGGERED\n");
    }
#endif //DEBUG_FINDA
    uint8_t rsp[maxMsgLen];
    uint8_t len = protocol.EncodeResponseReadFINDA(rq, mf::finda.Pressed(), rsp);
    WriteToUSART(rsp, len);
}

void ReportVersion(const mp::RequestMsg &rq) {
    uint8_t v = 0;

    switch (rq.value) {
    case 0:
        v = project_version_major;
        break;
    case 1:
        v = project_version_minor;
        break;
    case 2:
        v = project_version_revision;
        break;
    case 3:
        // @@TODO may be allow reporting uint16_t number of errors,
        // but anything beyond 255 errors means there is something seriously wrong with the MMU
        v = mg::globals.DriveErrors();
        break;
    default:
        v = 0;
        break;
    }

    uint8_t rsp[10];
    uint8_t len = protocol.EncodeResponseVersion(rq, v, rsp);
    WriteToUSART(rsp, len);
}

void ReportRunningCommand() {
    mp::ResponseMsgParamCodes commandStatus;
    uint16_t value = 0;
    switch (currentCommand->Error()) {
    case ErrorCode::RUNNING:
        commandStatus = mp::ResponseMsgParamCodes::Processing;
        value = (uint16_t)currentCommand->State();
        break;
    case ErrorCode::OK:
        commandStatus = mp::ResponseMsgParamCodes::Finished;
        break;
    default:
        commandStatus = mp::ResponseMsgParamCodes::Error;
        value = (uint16_t)currentCommand->Error();
        break;
    }

    uint8_t rsp[maxMsgLen];
    uint8_t len = protocol.EncodeResponseQueryOperation(currentCommandRq, commandStatus, value, rsp);
    WriteToUSART(rsp, len);
}

void PlanCommand(const mp::RequestMsg &rq) {
    if (currentCommand->State() == ProgressCode::OK) {
        // We are allowed to start a new command as the previous one is in the OK finished state
        // The previous command may be in an error state, but as long as it is in ProgressCode::OK (aka finished)
        // we are safe here. It is the responsibility of the printer to ask for a command error code
        // before issuing another one - if needed.
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
        case mp::RequestMsgCodes::Mode:
            currentCommand = &logic::setMode;
            break;
        default:
            currentCommand = &logic::noCommand;
            break;
        }
        currentCommandRq = rq; // save the Current Command Request for indentification of responses
        currentCommand->Reset(rq.value);
        ReportCommandAccepted(rq, mp::ResponseMsgParamCodes::Accepted);
    } else {
        ReportCommandAccepted(rq, mp::ResponseMsgParamCodes::Rejected);
    }
}

void ProcessRequestMsg(const mp::RequestMsg &rq) {
    switch (rq.code) {
    case mp::RequestMsgCodes::Button:
        // behave just like if the user pressed a button
        mui::userInput.ProcessMessage(rq.value);
        break;
    case mp::RequestMsgCodes::Finda:
        // immediately report FINDA status
        ReportFINDA(rq);
        break;
    case mp::RequestMsgCodes::Query:
        // immediately report progress of currently running command
        ReportRunningCommand();
        break;
    case mp::RequestMsgCodes::Reset:
        // immediately reset the board - there is no response in this case
        hal::watchdog::ConfigureWatchDog(1); // set the watchdog to the lowest possible timeout
        for (;;)
            ; // cycle indefinitely (i.e. let the watchdog reset the CPU)
        break;
    case mp::RequestMsgCodes::Version:
        ReportVersion(rq);
        break;
    case mp::RequestMsgCodes::Wait:
        break; // @@TODO - not used anywhere yet
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

void Panic(ErrorCode ec) {
    currentCommand->Panic(ec);
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
    mui::userInput.Step();
    currentCommand->Step();

    hal::watchdog::ResetWatchDog();
}

int main() {
    setup();
    sei(); ///enable interrupts
    for (;;) {
        loop();
    }
    return 0;
}
