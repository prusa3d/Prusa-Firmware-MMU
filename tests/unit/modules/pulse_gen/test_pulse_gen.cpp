#include "catch2/catch.hpp"
#include "pulse_gen.h"
#include "../pins.h"
#include <stdio.h>

using Catch::Matchers::Equals;
using namespace modules::pulse_gen;
using hal::tmc2130::MotorParams;

TEST_CASE("pulse_gen::basic", "[pulse_gen]") {
    MotorParams mp = {
        .idx = 0,
        .dirOn = config::idler.dirOn,
        .csPin = IDLER_CS_PIN,
        .stepPin = IDLER_STEP_PIN,
        .sgPin = IDLER_SG_PIN,
        .uSteps = config::idler.uSteps
    };

    for (int accel = 100; accel <= 5000; accel *= 2) {
        PulseGen pg;
        pg.SetAcceleration(accel);
        pg.Move(100, 100);

        unsigned long ts = 0;
        st_timer_t next;
        do {
            next = pg.Step(mp);
            printf("%lu %u\n", ts, next);
            ts += next;
        } while (next);

        printf("\n\n");
    }
}
