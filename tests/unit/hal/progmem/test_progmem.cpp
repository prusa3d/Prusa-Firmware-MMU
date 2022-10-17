#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"
#include "progmem.h"

using Catch::Matchers::Equals;
namespace pm = hal::progmem;

TEST_CASE("progmem::read_word", "[progmem]") {

    // create a PROGMEM array
    const uint16_t arr[2] PROGMEM = { 0, 1 };

    // ensure it can be read correctly
    REQUIRE(0 == pm::read_word(&arr[0]));
    REQUIRE(1 == pm::read_word(&arr[1]));
}
