#pragma once
#include <stdint.h>
#include "speed_table.h"
#include "../hal/tmc2130.h"
#include "../hal/circular_buffer.h"

namespace modules {

/// Acceleration ramp and stepper pulse generator
namespace pulse_gen {

using config::blockBufferSize;
using speed_table::st_timer_t;
typedef uint32_t steps_t; ///< Absolute step units
typedef uint32_t rate_t; ///< Type for step rates
typedef int32_t pos_t; ///< Axis position (signed)

class PulseGen {
public:
    PulseGen(steps_t max_jerk, steps_t acceleration);

    /// @returns the acceleration for the axis
    steps_t Acceleration() const { return acceleration; };

    /// Set acceleration for the axis
    void SetAcceleration(steps_t accel) { acceleration = accel; }

    /// Plan a single move (can only be executed when !Full())
    void Move(pos_t x, steps_t feed_rate);

    /// @returns the current position of the axis
    pos_t Position() const { return position; }

    /// Set the position of the axis
    void SetPosition(pos_t x) { position = x; }

    /// @returns true if all planned moves have been finished
    bool QueueEmpty() const { return block_index.empty(); }

    /// @returns false if new moves can still be planned
    bool Full() const { return block_index.full(); }

    /// Single-step the axis
    /// @returns the interval for the next tick
    st_timer_t Step(const hal::tmc2130::MotorParams &motorParams);

private:
    /// Motion parameters for the current planned or executing move
    struct block_t {
        steps_t steps; ///< Step events
        bool direction; ///< The direction for this block

        rate_t acceleration_rate; ///< The acceleration rate
        steps_t accelerate_until; ///< The index of the step event on which to stop acceleration
        steps_t decelerate_after; ///< The index of the step event on which to start decelerating

        // Settings for the trapezoid generator (runs inside an interrupt handler)
        rate_t nominal_rate; ///< The nominal step rate for this block in steps/sec
        rate_t initial_rate; ///< Rate at start of block
        rate_t final_rate; ///< Rate at exit
        rate_t acceleration; ///< acceleration steps/sec^2
    };

    // Block buffer parameters
    block_t block_buffer[blockBufferSize];
    CircularIndex<uint8_t, blockBufferSize> block_index;
    block_t *current_block;

    // Axis data
    pos_t position; ///< Current axis position
    steps_t max_jerk; ///< Axis jerk (could be constant)
    steps_t acceleration; ///< Current axis acceleration

    // Step parameters
    rate_t acceleration_time, deceleration_time;
    st_timer_t acc_step_rate; // decelaration start point
    uint8_t step_loops; // steps per loop
    uint8_t step_loops_nominal; // steps per loop at nominal speed
    st_timer_t timer_nominal; // nominal interval
    steps_t steps_completed; // steps completed

    /// Calculate the trapezoid parameters for the block
    void CalculateTrapezoid(block_t *block, steps_t entry_speed, steps_t exit_speed);
};

} // namespace pulse_gen
} // namespace modules
