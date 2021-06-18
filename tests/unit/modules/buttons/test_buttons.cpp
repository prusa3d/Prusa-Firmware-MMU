#include "catch2/catch.hpp"
#include "buttons.h"
#include "../stubs/stub_adc.h"

using Catch::Matchers::Equals;

uint16_t millis = 0;

bool Step_Basic_One_Button_Test(modules::buttons::Buttons &b, uint8_t oversampleFactor, uint8_t testedButtonIndex, uint8_t otherButton1, uint8_t otherButton2) {
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // should detect the press but remain in detected state - wait for debounce
    CHECK(!b.ButtonPressed(testedButtonIndex));
    CHECK(!b.ButtonPressed(otherButton1));
    CHECK(!b.ButtonPressed(otherButton2));

    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // reset to waiting
    CHECK(b.ButtonPressed(testedButtonIndex));
    CHECK(!b.ButtonPressed(otherButton1));
    CHECK(!b.ButtonPressed(otherButton2));

    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // pressed again, still in debouncing state
    CHECK(!b.ButtonPressed(testedButtonIndex));
    CHECK(!b.ButtonPressed(otherButton1));
    CHECK(!b.ButtonPressed(otherButton2));

    return true;
}

/// This test verifies the behaviour of a single button. The other buttons must remain intact.
bool Step_Basic_One_Button(hal::adc::TADCData &&d, uint8_t testedButtonIndex) {
    using namespace modules::buttons;
    millis = 0;
    Buttons b;

    // need to oversample the data as debouncing takes 100 cycles to accept a pressed button
    constexpr uint8_t oversampleFactor = 100;
    hal::adc::ReinitADC(0, std::move(d), oversampleFactor);

    uint8_t otherButton1 = 1, otherButton2 = 2;
    switch (testedButtonIndex) {
    case 1:
        otherButton1 = 0;
        break;
    case 2:
        otherButton2 = 0;
        break;
    default:
        break; // no change
    }

    return Step_Basic_One_Button_Test(b, oversampleFactor, testedButtonIndex, otherButton1, otherButton2);
}

TEST_CASE("buttons::Step-basic-button", "[buttons]") {
    {
        hal::adc::TADCData d({ 5, 6, 1023 });
        CHECK(Step_Basic_One_Button(std::move(d), 0));
    }
    {
        hal::adc::TADCData d({ 321, 359, 1023 });
        CHECK(Step_Basic_One_Button(std::move(d), 1));
    }
    {
        hal::adc::TADCData d({ 501, 529, 1023 });
        CHECK(Step_Basic_One_Button(std::move(d), 2));
    }
}

/// This test has to verify the independency of buttons - the ADC reads one button after the other
/// and the Buttons class should press first button and release, then the second one and then the third one
/// without being reinitialized.
TEST_CASE("buttons::Step-basic-button-one-after-other", "[buttons]") {
    using namespace modules::buttons;
    hal::adc::TADCData d({ 5, 6, 1023, 321, 359, 1023, 501, 529, 1023 });
    Buttons b;

    // need to oversample the data as debouncing takes 100 cycles to accept a pressed button
    constexpr uint8_t oversampleFactor = 100;
    hal::adc::ReinitADC(0, std::move(d), oversampleFactor);

    CHECK(Step_Basic_One_Button_Test(b, oversampleFactor, 0, 1, 2));
    CHECK(Step_Basic_One_Button_Test(b, oversampleFactor, 1, 0, 2));
    CHECK(Step_Basic_One_Button_Test(b, oversampleFactor, 2, 0, 1));
}

/// This test tries to simulate a bouncing effect on data from ADC on the first button
TEST_CASE("buttons::Step-debounce-one-button", "[buttons]") {
    using namespace modules::buttons;

    // make a bounce event on the first press
    hal::adc::TADCData d({ 5, 1023, 5, 9, 6, 7, 8, 1023, 1023 });

    // need to oversample the data as debouncing takes 100 cycles to accept a pressed button
    constexpr uint8_t oversampleFactor = 25;
    hal::adc::ReinitADC(0, std::move(d), oversampleFactor);

    Buttons b;

    // 5
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // should detect the press but remain in detected state - wait for debounce
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 1023
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // reset to waiting
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 5
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // pressed again, still in debouncing state
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 9
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // no change
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 6
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // no change
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 7
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // one step from "pressed"
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 8
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // fifth set of samples - should report "pressed" finally
    CHECK(b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 1023
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // sixth set of samples - button released (no debouncing on release)
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 1023
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(++millis); // seventh set of samples - still released
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));
}
