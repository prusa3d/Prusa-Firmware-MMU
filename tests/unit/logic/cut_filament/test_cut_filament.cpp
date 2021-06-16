#include "catch2/catch.hpp"

#include "../../../../src/modules/buttons.h"
#include "../../../../src/modules/finda.h"
#include "../../../../src/modules/fsensor.h"
#include "../../../../src/modules/globals.h"
#include "../../../../src/modules/idler.h"
#include "../../../../src/modules/leds.h"
#include "../../../../src/modules/permanent_storage.h"
#include "../../../../src/modules/selector.h"

#include "../../../../src/logic/cut_filament.h"

#include "../../modules/stubs/stub_adc.h"
#include "../stubs/main_loop_stub.h"

using Catch::Matchers::Equals;

TEST_CASE("cut_filament::basic1", "[cut_filament]") {
    using namespace logic;
    CutFilament cf;
    currentCommand = &cf;
    main_loop();
}
