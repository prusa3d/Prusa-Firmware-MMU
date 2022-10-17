#include "doctest/doctest.h"
#include <stdint.h>

TEST_CASE("type tests", "[system]") {
    REQUIRE(sizeof(uint64_t) == 8);
}
