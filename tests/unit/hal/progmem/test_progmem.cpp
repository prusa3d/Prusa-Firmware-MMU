#include "catch2/catch.hpp"
#include "progmem.h"

using Catch::Matchers::Equals;
using hal::progmem::pgm_read_word;

TEST_CASE("progmem::basic", "[progmem]") {

    // create a PROGMEM array
    const uint16_t arr[2] PROGMEM = {0, 1};

    // ensure it can be read correctly
    REQUIRE(0 == pgm_read_word(&arr[0]));
    REQUIRE(1 == pgm_read_word(&arr[1]));
}
