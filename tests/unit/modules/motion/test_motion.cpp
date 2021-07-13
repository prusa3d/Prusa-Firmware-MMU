#include "catch2/catch.hpp"
#include "motion.h"

using namespace modules::motion;

// Perform Step() until all moves are completed, returning the number of steps performed.
// Ensure the move doesn't run forever, making the test fail reliably.
ssize_t stepUntilDone(size_t maxSteps = 100000) {
    for (size_t i = 0; i != maxSteps; ++i)
        if (!motion.Step())
            return i;

    // number of steps exceeded
    return -1;
}

TEST_CASE("motion::basic", "[motion]") {
    // initial state
    REQUIRE(motion.QueueEmpty());
    REQUIRE(motion.Position(Idler) == 0);

    // enqueue a single move
    motion.PlanMoveTo(Idler, 10, 1);
    REQUIRE(!motion.QueueEmpty());

    // perform the move
    REQUIRE(stepUntilDone() == 10);
    REQUIRE(motion.QueueEmpty());

    // check positions
    REQUIRE(motion.Position(Idler) == 10);
}

TEST_CASE("motion::dual_move_fwd", "[motion]") {
    // check for configuration values that we cannot change but should match for this test
    // to function as expected (maybe this should be a static_assert?)
    REQUIRE(config::idler.jerk == config::selector.jerk);

    // enqueue moves on two axes
    REQUIRE(motion.QueueEmpty());

    // ensure the same acceleration is set on both
    motion.SetAcceleration(Idler, motion.Acceleration(Selector));
    REQUIRE(motion.Acceleration(Idler) == motion.Acceleration(Selector));

    // plan two moves at the same speed and acceleration
    motion.PlanMoveTo(Idler, 10, 1);
    motion.PlanMoveTo(Selector, 10, 1);

    // perform the move, which should be perfectly merged
    REQUIRE(stepUntilDone() == 10);
    REQUIRE(motion.QueueEmpty());

    // check for final axis positions
    REQUIRE(motion.Position(Idler) == 10);
    REQUIRE(motion.Position(Selector) == 10);
}

TEST_CASE("motion::dual_move_inv", "[motion]") {
    // check for configuration values that we cannot change but should match for this test
    // to function as expected (maybe this should be a static_assert?)
    REQUIRE(config::idler.jerk == config::selector.jerk);

    // enqueue moves on two axes
    REQUIRE(motion.QueueEmpty());

    // ensure the same acceleration is set on both
    motion.SetAcceleration(Idler, motion.Acceleration(Selector));
    REQUIRE(motion.Acceleration(Idler) == motion.Acceleration(Selector));

    // set two different starting points
    motion.SetPosition(Idler, 0);
    motion.SetPosition(Selector, 5);

    // plan two moves at the same speed and acceleration, like in the previous
    // test this should *also* reduce to the same steps being performed
    motion.PlanMove(Idler, 10, 1);
    motion.PlanMove(Selector, -10, 1);

    // perform the move, which should be perfectly merged
    REQUIRE(stepUntilDone() == 10);
    REQUIRE(motion.QueueEmpty());

    // check for final axis positions
    REQUIRE(motion.Position(Idler) == 10);
    REQUIRE(motion.Position(Selector) == -5);
}

TEST_CASE("motion::dual_move_complex", "[motion]") {
    // enqueue two completely different moves on two axes
    REQUIRE(motion.QueueEmpty());

    // set custom acceleration values
    motion.SetAcceleration(Idler, 10);
    motion.SetAcceleration(Selector, 20);

    // plan two moves at the same speed and acceleration, like in the previous
    // test: this should *also* reduce to the same steps being performed
    motion.PlanMoveTo(Idler, 10, 1);
    motion.PlanMoveTo(Selector, 10, 1);

    // perform the move, which should take less iterations than the sum of both
    REQUIRE(stepUntilDone(20));
    REQUIRE(motion.QueueEmpty());

    // check for final axis positions
    REQUIRE(motion.Position(Idler) == 10);
    REQUIRE(motion.Position(Selector) == 10);
}

TEST_CASE("motion::triple_move", "[motion]") {
    // that that we can move three axes at the same time
    motion.PlanMoveTo(Idler, 10, 1);
    motion.PlanMoveTo(Selector, 20, 1);
    motion.PlanMoveTo(Pulley, 30, 1);

    // perform the move with a maximum step limit
    REQUIRE(stepUntilDone(10 + 20 + 30));

    // check queue status
    REQUIRE(motion.QueueEmpty());

    // check for final axis positions
    REQUIRE(motion.Position(Idler) == 10);
    REQUIRE(motion.Position(Selector) == 20);
    REQUIRE(motion.Position(Pulley) == 30);
}
