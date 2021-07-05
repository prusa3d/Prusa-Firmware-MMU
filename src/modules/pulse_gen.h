#pragma once
#include <stdint.h>
#include "speed_table.h"
#include "../hal/tmc2130.h"

namespace modules {
namespace pulse_gen {

using speed_table::st_timer_t;
typedef uint32_t steps_t;
typedef uint32_t rate_t;
typedef int32_t pos_t;

struct block_t {
    // Fields used by the bresenham algorithm for tracing the line
    // steps_x.y,z, step_event_count, acceleration_rate, direction_bits and active_extruder are set by plan_buffer_line().
    steps_t step_event_count; // The number of step events required to complete this block
    rate_t acceleration_rate; // The acceleration rate used for acceleration calculation
    bool direction; // The direction bit set for this block (refers to *_DIRECTION_BIT in config.h)

    // accelerate_until and decelerate_after are set by calculate_trapezoid_for_block() and they need to be synchronized with the stepper interrupt controller.
    steps_t accelerate_until; // The index of the step event on which to stop acceleration
    steps_t decelerate_after; // The index of the step event on which to start decelerating

    // Fields used by the motion planner to manage acceleration
    //  float speed_x, speed_y, speed_z, speed_e;        // Nominal mm/sec for each axis
    // The nominal speed for this block in mm/sec.
    // This speed may or may not be reached due to the jerk and acceleration limits.
    float nominal_speed;
    // Entry speed at previous-current junction in mm/sec, respecting the acceleration and jerk limits.
    // The entry speed limit of the current block equals the exit speed of the preceding block.
    //float entry_speed;

    // The total travel of this block in mm
    float millimeters;
    // acceleration mm/sec^2
    float acceleration;

    // Settings for the trapezoid generator (runs inside an interrupt handler).
    // Changing the following values in the planner needs to be synchronized with the interrupt handler by disabling the interrupts.
    rate_t nominal_rate; // The nominal step rate for this block in step_events/sec
    rate_t initial_rate; // The jerk-adjusted step rate at start of block
    rate_t final_rate; // The minimal rate at exit
    rate_t acceleration_st; // acceleration steps/sec^2

    // Pre-calculated division for the calculate_trapezoid_for_block() routine to run faster.
    float speed_factor;
};

class PulseGen {
public:
    PulseGen();

    float Acceleration() const { return acceleration; };
    void SetAcceleration(float accel) { acceleration = accel; }

    void Move(float x, float feed_rate);
    float Position() const;
    bool QueueEmpty() const;
    bool Full() const;

    st_timer_t Step(const hal::tmc2130::MotorParams &motorParams);

private:
    //{ units constants
    steps_t axis_steps_per_sqr_second;
    float axis_steps_per_unit;
    float max_jerk;
    steps_t dropsegments; // segments are dropped if lower than that
    //}

    //{ block buffer
    block_t block_buffer[2];
    block_t *current_block;
    uint8_t block_buffer_head;
    uint8_t block_buffer_tail;
    //}

    //{ state
    pos_t position;
    float acceleration;

    rate_t acceleration_time, deceleration_time;
    st_timer_t acc_step_rate; // decelaration start point
    uint8_t step_loops;
    uint8_t step_loops_nominal;
    st_timer_t timer_nominal;
    steps_t step_events_completed;
    //}

    void calculate_trapezoid_for_block(block_t *block, float entry_speed, float exit_speed);
};

} // namespace pulse_gen
} // namespace modules
