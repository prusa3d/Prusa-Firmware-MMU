#include "catch2/catch.hpp"

#include "../../../../src/modules/buttons.h"
#include "../../../../src/modules/finda.h"
#include "../../../../src/modules/fsensor.h"
#include "../../../../src/modules/globals.h"
#include "../../../../src/modules/idler.h"
#include "../../../../src/modules/leds.h"
#include "../../../../src/modules/motion.h"
#include "../../../../src/modules/permanent_storage.h"
#include "../../../../src/modules/selector.h"

#include "../../../../src/logic/tool_change.h"

#include "../../modules/stubs/stub_adc.h"

#include "../stubs/main_loop_stub.h"
#include "../stubs/stub_motion.h"

using Catch::Matchers::Equals;

namespace mm = modules::motion;
namespace mf = modules::finda;
namespace mi = modules::idler;
namespace ml = modules::leds;
namespace mb = modules::buttons;
namespace mg = modules::globals;
namespace ms = modules::selector;

TEST_CASE("tool_change::test0", "[tool_change]") {
    using namespace logic;

    ForceReinitAllAutomata();

    ToolChange tc;
    // restart the automaton
    tc.Reset(0);

    main_loop();

    REQUIRE(WhileTopState(tc, ProgressCode::UnloadingFilament, 5000));
    REQUIRE(modules::globals::globals.FilamentLoaded() == false);

    REQUIRE(tc.TopLevelState() == ProgressCode::LoadingFilament);
    REQUIRE(WhileTopState(tc, ProgressCode::LoadingFilament, 5000));

    REQUIRE(tc.TopLevelState() == ProgressCode::OK);
    REQUIRE(modules::globals::globals.FilamentLoaded() == true);
}
