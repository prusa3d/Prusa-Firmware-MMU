#include "catch2/catch.hpp"
#include "application.h"
#include <stdint.h>
#include "../modules/stubs/stub_serial.h"

TEST_CASE("application::ReportVersion", "[application]") {
    modules::serial::ClearRX();
    modules::serial::ClearTX();

    application.ReportReadRegister(mp::RequestMsg(mp::RequestMsgCodes::Version, 0));

    REQUIRE(modules::serial::tx == "S0 A2*37\n");
}
