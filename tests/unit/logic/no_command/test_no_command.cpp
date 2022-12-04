#include "catch2/catch_test_macros.hpp"

#include "../../../../src/modules/buttons.h"
#include "../../../../src/modules/finda.h"
#include "../../../../src/modules/fsensor.h"
#include "../../../../src/modules/globals.h"
#include "../../../../src/modules/idler.h"
#include "../../../../src/modules/leds.h"
#include "../../../../src/modules/motion.h"
#include "../../../../src/modules/permanent_storage.h"
#include "../../../../src/modules/selector.h"
#include "../../../../src/modules/pulley.h"
#include "../../../../src/modules/user_input.h"

#include "../../../../src/logic/no_command.h"

#include "../../modules/stubs/stub_adc.h"

#include "../stubs/homing.h"
#include "../stubs/main_loop_stub.h"
#include "../stubs/stub_motion.h"

#include "../helpers/helpers.ipp"

TEST_CASE("no_command::SetInitError", "[no_command]") {
    logic::NoCommand nc;

    // Step 1 - Check there are no errors
    REQUIRE(nc.Error() == ErrorCode::OK);
    REQUIRE(nc.State() == ProgressCode::OK);

    // Step 2 - Create error
    nc.SetInitError(ErrorCode::FINDA_VS_EEPROM_DISREPANCY);

    // Step 3 - Check error is waiting for user input
    REQUIRE(nc.Error() == ErrorCode::FINDA_VS_EEPROM_DISREPANCY);
    REQUIRE(nc.State() == ProgressCode::ERRWaitingForUser);

    // Step 4 - Loop through a few iterations, error remains unchanged
    for (size_t i = 0; i < 100; ++i) {
        nc.Step();
    }

    REQUIRE(nc.Error() == ErrorCode::FINDA_VS_EEPROM_DISREPANCY);
    REQUIRE(nc.State() == ProgressCode::ERRWaitingForUser);
}

TEST_CASE("no_command::FINDA_VS_EEPROM_DISREPANCY_retry", "[no_command]") {
    logic::NoCommand nc;

    // Initalise the ADC
    hal::adc::SetADC(config::buttonsADCIndex, config::buttonADCMaxValue);

    // Set FINDA and Fsensor on
    SetFSensorStateAndDebounce(true);
    SetFINDAStateAndDebounce(true);

    // Step 1 - Check there are no errors
    REQUIRE(nc.Error() == ErrorCode::OK);
    REQUIRE(nc.State() == ProgressCode::OK);

    // Step 2 - Create error
    nc.SetInitError(ErrorCode::FINDA_VS_EEPROM_DISREPANCY);

    // Step 3 - Check error is waiting for user input
    REQUIRE(nc.Error() == ErrorCode::FINDA_VS_EEPROM_DISREPANCY);
    REQUIRE(nc.State() == ProgressCode::ERRWaitingForUser);

    // Step 4 - Press and release button (from Printer)
    PressButtonAndDebounce(nc, mb::Middle, true);
    ClearButtons(nc);

    // Check that the error is still present
    REQUIRE(nc.Error() == ErrorCode::FINDA_VS_EEPROM_DISREPANCY);
    REQUIRE(nc.State() == ProgressCode::ERRWaitingForUser);

    // Untrigger FINDA and FSensor
    SetFSensorStateAndDebounce(false);
    SetFINDAStateAndDebounce(false);

    // Step 4 - Press and release button
    PressButtonAndDebounce(nc, mb::Middle, true);
    ClearButtons(nc);

    // Now the error should be gone :)
    REQUIRE(nc.Error() == ErrorCode::OK);
    REQUIRE(nc.State() == ProgressCode::OK);
}
