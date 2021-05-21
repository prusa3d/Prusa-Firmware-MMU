#include "catch2/catch.hpp"
#include "circular_buffer.h"

using Catch::Matchers::Equals;

TEST_CASE("circular_buffer::basic", "[circular_buffer]") {

    using CB = CircularBuffer<uint8_t, uint8_t, 32>;

    CB cb;

    // at the beginning the buffer is empty
    REQUIRE(cb.empty());

    // since its capacity was defined as 32, at least one element must be successfully inserted
    CHECK(cb.push(1));

    // is the element there?
    REQUIRE(!cb.empty());
    CHECK(cb.front() == 1);

    // remove the element
    uint8_t b = 0;
    CHECK(cb.pop(b));
    CHECK(b == 1);
    CHECK(cb.empty());
}
