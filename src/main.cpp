/// @file main.cpp
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
#include "modules/pulley.h"
#include "modules/selector.h"
#include "modules/user_input.h"
#include "modules/timebase.h"
#include "modules/motion.h"
#include "modules/usb_cdc.h"

#include "application.h"

#include "logic/hw_sanity.h"
#include "logic/no_command.h"

/// One-time setup of HW and SW components
/// Called before entering the loop() function
/// Green LEDs signalize the progress of initialization. If anything goes wrong we shall turn on a red LED
/// Executed with interrupts disabled
static void setup() {
    hal::cpu::Init();

    mt::timebase.Init();

    // watchdog init
    hwd::Enable(hwd::configuration::compute(8000)); //set 8s timeout

    mg::globals.Init();

    hal::shr16::shr16.Init();
    ml::leds.SetMode(4, ml::green, ml::on);
    ml::leds.Step();

    // if the shift register doesn't work we really can't signalize anything, only internal variables will be accessible if the UART works
    hu::USART::USART_InitTypeDef usart_conf = {
        .rx_pin = USART_RX,
        .tx_pin = USART_TX,
        .baudrate = 115200,
    };
    hu::usart1.Init(&usart_conf);
    ml::leds.SetMode(3, ml::green, ml::on);
    ml::leds.Step();

    // @@TODO if both shift register and the UART are dead, we are sitting ducks :(

    hal::spi::SPI_InitTypeDef spi_conf = {
        .miso_pin = TMC2130_SPI_MISO_PIN,
        .mosi_pin = TMC2130_SPI_MOSI_PIN,
        .sck_pin = TMC2130_SPI_SCK_PIN,
        .ss_pin = TMC2130_SPI_SS_PIN,
        .prescaler = 2, //4mhz
        .cpha = 1,
        .cpol = 1,
    };
    hal::spi::Init(SPI0, &spi_conf);
    ml::leds.SetMode(2, ml::green, ml::on);
    ml::leds.Step();

    mm::Init();
    ml::leds.SetMode(1, ml::green, ml::on);
    ml::leds.Step();

    ha::Init();
    ml::leds.SetMode(0, ml::green, ml::on);
    ml::leds.Step();

    mu::cdc.Init();
}

/// Second part of setup - executed with interrupts enabled
static void setup2() {
    // waits at least finda debounce period
    // which is abused to let the LEDs shine for ~100ms
    mf::finda.BlockingInit();

    if (mf::finda.Pressed() && mg::globals.FilamentLoaded() < mg::InFSensor) {
        // This is a tricky situation - EEPROM doesn't have a record about loaded filament (blocking the Selector)
        // To be on the safe side, we have to override the EEPROM information about filament position - at least InFSensor
        // Moreover - we need to override the active slot position as well, because we cannot know where the Selector is.
        // For this we speculatively set the active slot to 2 (in the middle ;) )
        // Ideally this should be signalled as an error state and displayed on the printer and recovered properly.
        mg::globals.SetFilamentLoaded(2, mg::InFSensor);
        logic::noCommand.SetInitError(ErrorCode::FINDA_VS_EEPROM_DISREPANCY);
    } else if (!mf::finda.Pressed() && mg::globals.FilamentLoaded() >= mg::InSelector) {
        // Opposite situation - not so dangerous, but definitely confusing to users.
        // FINDA is not pressed but we have a record in the EEPROM.
        // It has been decided, that we shall clear such a record from EEPROM automagically
        // and presume there is no filament at all (requires working FINDA)
        mg::globals.SetFilamentLoaded(config::toolCount, mg::AtPulley);
    }

    /// Turn off all leds
    for (uint8_t i = 0; i < config::toolCount; i++) {
        ml::leds.SetMode(i, ml::green, ml::off);
        ml::leds.SetMode(i, ml::red, ml::off);
    }
    ml::leds.Step();

    // Prep hardware sanity:
    logic::hwSanity.Reset(0);

    while (!logic::hwSanity.StepInner()) {
        ml::leds.Step();
    }

    if (logic::hwSanity.Error() != ErrorCode::OK) {
        // forward the issue to the logic startup handler.
        logic::noCommand.SetInitError(logic::hwSanity.Error());
        return;
    }

    // Idler and Selector decide whether homing is possible/safe
    mi::idler.Init();
    ms::selector.Init();

    // activate the correct LED if filament is present
    if (mg::globals.FilamentLoaded() > mg::FilamentLoadState::AtPulley) {
        ml::leds.SetMode(mg::globals.ActiveSlot(), ml::green, ml::on);
    }
}

void Panic(ErrorCode ec) {
    application.Panic(ec);
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

    mb::buttons.Step();
    ml::leds.Step();
    if (logic::hwSanity.Error() == ErrorCode::OK) {
        mf::finda.Step();
        mfs::fsensor.Step();
        mi::idler.Step();
        ms::selector.Step();
        mpu::pulley.Step();
        mui::userInput.Step();
    }
    hal::cpu::Step();
    mu::cdc.Step();

    application.Step();

    hal::watchdog::Reset();
}

int main() {
    setup();
    sei(); ///enable interrupts
    setup2();
    for (;;) {
        loop();
    }
    return 0;
}

// avoid avr-gcc 5.4 link time code generation bug
extern "C" void __cxa_pure_virtual() {
    while (1)
        ;
}
