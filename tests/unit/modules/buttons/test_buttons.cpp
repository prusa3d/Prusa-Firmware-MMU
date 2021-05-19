#include "catch2/catch.hpp"
#include "buttons.h"
#include "stub_adc.h"

using Catch::Matchers::Equals;

bool Step_Basic_One_Button_Test(modules::Buttons &b, uint8_t oversampleFactor, uint8_t testedButtonIndex, uint8_t otherButton1, uint8_t otherButton2) {
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // should detect the press but remain in detected state - wait for debounce
    CHECK(!b.ButtonPressed(testedButtonIndex));
    CHECK(!b.ButtonPressed(otherButton1));
    CHECK(!b.ButtonPressed(otherButton2));

    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // reset to waiting
    CHECK(b.ButtonPressed(testedButtonIndex));
    CHECK(!b.ButtonPressed(otherButton1));
    CHECK(!b.ButtonPressed(otherButton2));

    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // pressed again, still in debouncing state
    CHECK(!b.ButtonPressed(testedButtonIndex));
    CHECK(!b.ButtonPressed(otherButton1));
    CHECK(!b.ButtonPressed(otherButton2));

    return true;
}

/// This test verifies the behaviour of a single button. The other buttons must remain intact.
bool Step_Basic_One_Button(hal::ADC::TADCData &&d, uint8_t testedButtonIndex) {
    using namespace modules;

    Buttons b;

    // need to oversample the data as debouncing takes 100 cycles to accept a pressed button
    constexpr uint8_t oversampleFactor = 100;
    hal::ADC::ReinitADC(std::move(d), oversampleFactor);

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
        hal::ADC::TADCData d({ 5, 6, 1023 });
        CHECK(Step_Basic_One_Button(std::move(d), 0));
    }
    {
        hal::ADC::TADCData d({ 321, 359, 1023 });
        CHECK(Step_Basic_One_Button(std::move(d), 1));
    }
    {
        hal::ADC::TADCData d({ 501, 529, 1023 });
        CHECK(Step_Basic_One_Button(std::move(d), 2));
    }
}

/// This test has to verify the independency of buttons - the ADC reads one button after the other
/// and the Buttons class should press first button and release, then the second one and then the third one
/// without being reinitialized.
TEST_CASE("buttons::Step-basic-button-one-after-other", "[buttons]") {
    using namespace modules;
    hal::ADC::TADCData d({ 5, 6, 1023, 321, 359, 1023, 501, 529, 1023 });
    Buttons b;

    // need to oversample the data as debouncing takes 100 cycles to accept a pressed button
    constexpr uint8_t oversampleFactor = 100;
    hal::ADC::ReinitADC(std::move(d), oversampleFactor);

    CHECK(Step_Basic_One_Button_Test(b, oversampleFactor, 0, 1, 2));
    CHECK(Step_Basic_One_Button_Test(b, oversampleFactor, 1, 0, 2));
    CHECK(Step_Basic_One_Button_Test(b, oversampleFactor, 2, 0, 1));
}

/// This test tries to simulate a bouncing effect on data from ADC on the first button
TEST_CASE("buttons::Step-debounce-one-button", "[buttons]") {
    using namespace modules;

    // make a bounce event on the first press
    hal::ADC::TADCData d({ 5, 1023, 5, 9, 6, 7, 8, 1023, 1023 });

    // need to oversample the data as debouncing takes 100 cycles to accept a pressed button
    constexpr uint8_t oversampleFactor = 25;
    hal::ADC::ReinitADC(std::move(d), oversampleFactor);

    Buttons b;

    // 5
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // should detect the press but remain in detected state - wait for debounce
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 1023
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // reset to waiting
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 5
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // pressed again, still in debouncing state
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 9
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // no change
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 6
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // no change
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 7
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // one step from "pressed"
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 8
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // fifth set of samples - should report "pressed" finally
    CHECK(b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 1023
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // sixth set of samples - button released (no debouncing on release)
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));

    // 1023
    for (uint8_t i = 0; i < oversampleFactor; ++i)
        b.Step(); // seventh set of samples - still released
    CHECK(!b.ButtonPressed(0));
    CHECK(!b.ButtonPressed(1));
    CHECK(!b.ButtonPressed(2));
}
