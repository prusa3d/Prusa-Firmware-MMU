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

TEST_CASE("motion::unit", "[motion]") {
    // test AxisUnit conversion in the PlanMove and PlanMoveTo.
    using config::operator"" _mm;
    using config::operator"" _mm_s;
    using config::operator"" _deg;
    using config::operator"" _deg_s;

    REQUIRE(motion.QueueEmpty());
    REQUIRE(motion.Position(Pulley) == 0);

    // move with AxisUnit
    pos_t target = config::pulley.stepsPerUnit * 10;
    motion.PlanMoveTo<Pulley>(10.0_P_mm, 100.0_P_mm_s);
    CHECK(stepUntilDone());
    REQUIRE(motion.Position(Pulley) == target);

    // move directly with physical units
    motion.PlanMoveTo<Pulley>(10.0_mm, 100.0_mm_s);
    REQUIRE(stepUntilDone() == 0);
    REQUIRE(motion.Position(Pulley) == target);

    // relative move with AxisUnit
    motion.PlanMove<Pulley>(-5.0_P_mm, 100.0_P_mm_s);
    CHECK(stepUntilDone());
    REQUIRE(motion.Position(Pulley) == target / 2);

    // relative move with physical unit
    motion.PlanMove<Pulley>(-5.0_mm, 100.0_mm_s);
    CHECK(stepUntilDone());
    REQUIRE(motion.Position(Pulley) == 0);

    // now test remaining axes
    target = config::selector.stepsPerUnit * 10;
    motion.PlanMoveTo<Selector>(10.0_S_mm, 100.0_S_mm_s);
    motion.PlanMove<Selector>(10.0_mm, 100.0_mm_s);
    CHECK(stepUntilDone());
    REQUIRE(motion.Position(Selector) == target * 2);

    target = config::idler.stepsPerUnit * 10;
    motion.PlanMoveTo<Idler>(10.0_I_deg, 100.0_I_deg_s);
    motion.PlanMove<Idler>(10.0_deg, 100.0_deg_s);
    CHECK(stepUntilDone());
    REQUIRE(motion.Position(Idler) == target * 2);
}

TEST_CASE("motion::dual_move_fwd", "[motion]") {
    // enqueue moves on two axes
    REQUIRE(motion.QueueEmpty());

    // ensure the same jerk is set on both
    motion.SetJerk(Idler, motion.Jerk(Selector));
    REQUIRE(motion.Jerk(Idler) == motion.Jerk(Selector));

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
    // enqueue moves on two axes
    REQUIRE(motion.QueueEmpty());

    // ensure the same jerk is set on both
    motion.SetJerk(Idler, motion.Jerk(Selector));
    REQUIRE(motion.Jerk(Idler) == motion.Jerk(Selector));

    // ensure the same acceleration is set on both
    motion.SetAcceleration(Idler, motion.Acceleration(Selector));
    REQUIRE(motion.Acceleration(Idler) == motion.Acceleration(Selector));

    // set two different starting points
    motion.SetPosition(Idler, 0);
    motion.SetPosition(Selector, 5);

    // plan two moves at the same speed and acceleration: like in the previous
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

    // plan two moves with difference accelerations
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
    // check that we can move three axes at the same time
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

TEST_CASE("motion::dual_move_ramp", "[motion]") {
    // TODO: output ramps still to be checked
    const int idlerSteps = 100;
    const int selectorSteps = 80;
    const int maxFeedRate = 1000;

    for (int accel = 2000; accel <= 50000; accel *= 2) {
        REQUIRE(motion.QueueEmpty());

        // first axis using nominal values
        motion.SetPosition(Idler, 0);
        motion.SetAcceleration(Idler, accel);
        motion.PlanMoveTo(Idler, idlerSteps, maxFeedRate);

        // second axis finishes slightly sooner at triple acceleration to maximize the
        // aliasing effects
        motion.SetPosition(Selector, 0);
        motion.SetAcceleration(Selector, accel * 3);
        motion.PlanMoveTo(Selector, selectorSteps, maxFeedRate);

        // step and output time, interval and positions
        unsigned long ts = 0;
        st_timer_t next;
        do {
            next = motion.Step();
            pos_t pos_idler = motion.CurPosition(Idler);
            pos_t pos_selector = motion.CurPosition(Selector);

            printf("%lu %u %d %d\n", ts, next, pos_idler, pos_selector);

            ts += next;
        } while (next);
        printf("\n\n");

        // check queue status
        REQUIRE(motion.QueueEmpty());

        // check final position
        REQUIRE(motion.Position(Idler) == idlerSteps);
        REQUIRE(motion.Position(Selector) == selectorSteps);
    }
}
