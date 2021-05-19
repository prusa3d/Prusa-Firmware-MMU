#include "catch2/catch.hpp"
#include "buttons.h"
#include "stub_adc.h"

using Catch::Matchers::Equals;

TEST_CASE("buttons::Step", "[buttons]") {
    using namespace modules;

    hal::ADC::TADCData d = { 1, 2, 3, 4, 5 };
    hal::ADC::ReinitADC(d);

    Buttons b;
    b.Step();
}
