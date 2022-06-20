#include "catch2/catch.hpp"
#include "../stubs/stub_adc.h"
#include "../stubs/stub_timebase.h"
#include "buttons.h"
#include "../hal/adc.h"
#include "user_input.h"

void PressButtonAndDebounce(uint8_t btnIndex) {
    hal::adc::SetADC(config::buttonsADCIndex, config::buttonADCLimits[btnIndex][0] + 1);
    while (!mb::buttons.ButtonPressed(btnIndex)) {
        mb::buttons.Step();
        mui::userInput.Step();
        mt::IncMillis();
    }
}

TEST_CASE("user_input::printer_in_charge", "[user_input]") {
    uint8_t button;
    mui::Event event;
    std::tie(button, event) = GENERATE(
        std::make_tuple(mb::Left, mui::Event::Left),
        std::make_tuple(mb::Middle, mui::Event::Middle),
        std::make_tuple(mb::Right, mui::Event::Right));

    mt::ReinitTimebase();
    mb::Buttons b;
    // reset UI
    new (&mui::userInput) mui::UserInput();
    REQUIRE_FALSE(mui::userInput.PrinterInCharge());

    // set printer in charge
    mui::userInput.SetPrinterInCharge(true);
    REQUIRE(mui::userInput.PrinterInCharge());

    // put some button into the buffer - should be marked as "from printer"
    mui::userInput.ProcessMessage(button);
    // i.e. we should NOT be able to extract that message with ConsumeEventForPrinter()
    REQUIRE(mui::userInput.ConsumeEventForPrinter() == mui::NoEvent);
    REQUIRE(mui::userInput.AnyEvent());
    // but we should be able to extract that message with ConsumeEvent()
    REQUIRE(mui::userInput.ConsumeEvent() == event);
    REQUIRE_FALSE(mui::userInput.AnyEvent());

    // press a button on the MMU
    PressButtonAndDebounce(button);
    REQUIRE(mb::buttons.ButtonPressed(button));
    // we should NOT be able to extract the event with ConsumeEvent
    REQUIRE(mui::userInput.ConsumeEvent() == mui::NoEvent);
    REQUIRE(mui::userInput.AnyEvent());
    // but we should be able to extract that message with ConsumeEventForPrinter
    REQUIRE(mui::userInput.ConsumeEventForPrinter() == event);
    REQUIRE_FALSE(mui::userInput.AnyEvent());
}

TEST_CASE("user_input::button_pressed_MMU", "[user_input]") {
    uint8_t button;
    mui::Event event;
    std::tie(button, event) = GENERATE(
        std::make_tuple(mb::Left, mui::Event::Left),
        std::make_tuple(mb::Middle, mui::Event::Middle),
        std::make_tuple(mb::Right, mui::Event::Right));

    // create a button press
    mt::ReinitTimebase();
    mb::Buttons b;

    // reset UI
    new (&mui::userInput) mui::UserInput();
    REQUIRE_FALSE(mui::userInput.PrinterInCharge());

    PressButtonAndDebounce(button);
    REQUIRE(mb::buttons.ButtonPressed(button));
    // we should be able to extract the event with ConsumeEvent
    REQUIRE(mui::userInput.AnyEvent());
    REQUIRE(mui::userInput.ConsumeEvent() == event);
    REQUIRE_FALSE(mui::userInput.AnyEvent());
}

TEST_CASE("user_input::empty_queue", "[user_input]") {
    mt::ReinitTimebase();
    mb::Buttons b;

    // reset UI
    new (&mui::userInput) mui::UserInput();
    REQUIRE_FALSE(mui::userInput.PrinterInCharge());

    REQUIRE_FALSE(mui::userInput.AnyEvent());
    // try extracting something
    REQUIRE(mui::userInput.ConsumeEvent() == mui::NoEvent);
    REQUIRE(mui::userInput.ConsumeEventForPrinter() == mui::NoEvent);
}
