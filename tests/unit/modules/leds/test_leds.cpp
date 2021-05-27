#include "catch2/catch.hpp"
#include "leds.h"
#include "shr16.h"

using Catch::Matchers::Equals;

namespace hal {
namespace shr16 {
extern uint16_t shr16_v_copy;
} // namespace shr16
} // namespace hal

/// LEDS - hardcoded
#define SHR16_LEDG0 0x0100
#define SHR16_LEDR0 0x0200
#define SHR16_LEDG1 0x0400
#define SHR16_LEDR1 0x0800
#define SHR16_LEDG2 0x1000
#define SHR16_LEDR2 0x2000
#define SHR16_LEDG3 0x4000
#define SHR16_LEDR3 0x8000
#define SHR16_LEDG4 0x0040
#define SHR16_LEDR4 0x0080

TEST_CASE("leds::single", "[leds]") {
    using namespace modules::leds;
    using namespace hal::shr16;

    LEDs leds;

    uint8_t index;
    Color color;
    uint16_t shr16_register;
    std::tie(index, color, shr16_register) = GENERATE(
        std::make_tuple(0, green, SHR16_LEDG0),
        std::make_tuple(0, red, SHR16_LEDR0),
        std::make_tuple(1, green, SHR16_LEDG1),
        std::make_tuple(1, red, SHR16_LEDR1),
        std::make_tuple(2, green, SHR16_LEDG2),
        std::make_tuple(2, red, SHR16_LEDR2),
        std::make_tuple(3, green, SHR16_LEDG3),
        std::make_tuple(3, red, SHR16_LEDR3),
        std::make_tuple(4, green, SHR16_LEDG4),
        std::make_tuple(4, red, SHR16_LEDR4));

    shr16.Init(); // clears the register for the test

    // turn LED on
    leds.SetMode(index, color, on);
    leds.Step(0);
    CHECK(leds.LedOn(index, color) == true);
    CHECK(shr16_v_copy == shr16_register);

    // turn LED off
    leds.SetMode(index, color, off);
    leds.Step(0);
    CHECK(leds.LedOn(index, color) == false);
    CHECK(shr16_v_copy == 0);
}

void TestBlink(uint8_t index, modules::leds::Color color, uint16_t shr16_register, bool shouldBeOn, modules::leds::Mode blinkMode) {
    using namespace modules::leds;
    using namespace hal::shr16;
    LEDs leds;

    leds.SetMode(index, color, blinkMode);
    leds.Step(1);

    REQUIRE(leds.LedOn(index, color) == shouldBeOn);
    CHECK(shr16_v_copy == (shouldBeOn ? shr16_register : 0));

    // test 4 seconds of blinking
    for (uint8_t s = 1; s < 4; ++s) {
        // one second elapsed ;)
        leds.Step(1000);
        shouldBeOn = !shouldBeOn;
        CHECK(leds.LedOn(index, color) == shouldBeOn);
        CHECK(shr16_v_copy == (shouldBeOn ? shr16_register : 0));
    }

    // turn LED off
    leds.SetMode(index, color, off);
    leds.Step(0);
    CHECK(leds.LedOn(index, color) == false);
    CHECK(shr16_v_copy == 0);
}

TEST_CASE("leds::blink0-single", "[leds]") {
    using namespace modules::leds;
    using namespace hal::shr16;

    uint8_t index;
    Color color;
    uint16_t shr16_register;
    std::tie(index, color, shr16_register) = GENERATE(
        std::make_tuple(0, green, SHR16_LEDG0),
        std::make_tuple(0, red, SHR16_LEDR0),
        std::make_tuple(1, green, SHR16_LEDG1),
        std::make_tuple(1, red, SHR16_LEDR1),
        std::make_tuple(2, green, SHR16_LEDG2),
        std::make_tuple(2, red, SHR16_LEDR2),
        std::make_tuple(3, green, SHR16_LEDG3),
        std::make_tuple(3, red, SHR16_LEDR3),
        std::make_tuple(4, green, SHR16_LEDG4),
        std::make_tuple(4, red, SHR16_LEDR4));

    shr16.Init(); // clears the register for the test

    // set LED into blink0 mode - on in even periods of 1s intervals, starts as OFF
    TestBlink(index, color, shr16_register, false, blink0);

    TestBlink(index, color, shr16_register, true, blink1);
}
