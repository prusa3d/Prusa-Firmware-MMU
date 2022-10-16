#include "catch2/catch_all.hpp"

TEST_CASE("type tests", "[system]") {
    REQUIRE(sizeof(uint64_t) == 8);
}
