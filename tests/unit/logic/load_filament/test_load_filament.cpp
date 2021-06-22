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

#include "../../../../src/logic/load_filament.h"

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

TEST_CASE("unload_filament::unload0", "[unload_filament]") {
    using namespace logic;

    ForceReinitAllAutomata();

    LoadFilament lf;
    // restart the automaton
    lf.Reset(0);

    main_loop();

    //    REQUIRE(WhileCondition([&]() { return uf.TopLevelState() == ProgressCode::UnloadingToFinda; }, 5000));

    //    REQUIRE(uf.TopLevelState() == ProgressCode::DisengagingIdler);
    //    REQUIRE(WhileCondition([&]() { return uf.TopLevelState() == ProgressCode::DisengagingIdler; }, 5000));

    //    CHECK(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(5));

    //    REQUIRE(uf.TopLevelState() == ProgressCode::AvoidingGrind);
    //    REQUIRE(WhileCondition([&]() { return uf.TopLevelState() == ProgressCode::AvoidingGrind; }, 5000));

    //    REQUIRE(uf.TopLevelState() == ProgressCode::FinishingMoves);
    //    REQUIRE(WhileCondition([&]() { return uf.TopLevelState() == ProgressCode::FinishingMoves; }, 5000));

    //    REQUIRE(uf.TopLevelState() == ProgressCode::OK);
    REQUIRE(modules::globals::globals.FilamentLoaded() == true);
}
