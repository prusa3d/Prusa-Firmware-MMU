#include "pulse_gen.h"
using hal::tmc2130::MotorParams;
using hal::tmc2130::TMC2130;
using modules::math::mulU24X24toH16;
using modules::speed_table::calc_timer;

#include "../cmath.h"

namespace modules {
namespace pulse_gen {

PulseGen::PulseGen() {
    // Some initial values
    position = 0;
    acceleration = 1200;
    block_buffer_head = block_buffer_tail = 0;
    current_block = nullptr;

    // TODO: configuration constants
    dropsegments = 5;
    max_jerk = 100;
}

void PulseGen::CalculateTrapezoid(block_t *block, steps_t entry_speed, steps_t exit_speed) {
    // Minimum stepper rate 120Hz, maximum 40kHz. If the stepper rate goes above 10kHz,
    // the stepper interrupt routine groups the pulses by 2 or 4 pulses per interrupt tick.
    rate_t initial_rate = entry_speed;
    rate_t final_rate = exit_speed;

    // Limit minimal step rate (Otherwise the timer will overflow.)
    if (initial_rate < MINIMAL_STEP_RATE)
        initial_rate = MINIMAL_STEP_RATE;
    if (initial_rate > block->nominal_rate)
        initial_rate = block->nominal_rate;
    if (final_rate < MINIMAL_STEP_RATE)
        final_rate = MINIMAL_STEP_RATE;
    if (final_rate > block->nominal_rate)
        final_rate = block->nominal_rate;

    // Don't allow zero acceleration.
    rate_t acceleration = block->acceleration;
    if (acceleration == 0)
        acceleration = 1;

    // estimate_acceleration_distance(float initial_rate, float target_rate, float acceleration)
    // (target_rate*target_rate-initial_rate*initial_rate)/(2.0*acceleration));
    rate_t initial_rate_sqr = initial_rate * initial_rate;
    rate_t nominal_rate_sqr = block->nominal_rate * block->nominal_rate;
    rate_t final_rate_sqr = final_rate * final_rate;
    rate_t acceleration_x2 = acceleration << 1;
    // ceil(estimate_acceleration_distance(initial_rate, block->nominal_rate, acceleration));
    steps_t accelerate_steps = (nominal_rate_sqr - initial_rate_sqr + acceleration_x2 - 1) / acceleration_x2;
    // floor(estimate_acceleration_distance(block->nominal_rate, final_rate, -acceleration));
    steps_t decelerate_steps = (nominal_rate_sqr - final_rate_sqr) / acceleration_x2;
    steps_t accel_decel_steps = accelerate_steps + decelerate_steps;
    // Size of Plateau of Nominal Rate.
    steps_t plateau_steps = 0;

    // Is the Plateau of Nominal Rate smaller than nothing? That means no cruising, and we will
    // have to use intersection_distance() to calculate when to abort acceleration and start braking
    // in order to reach the final_rate exactly at the end of this block.
    if (accel_decel_steps < block->steps) {
        plateau_steps = block->steps - accel_decel_steps;
    } else {
        uint32_t acceleration_x4 = acceleration << 2;
        // Avoid negative numbers
        if (final_rate_sqr >= initial_rate_sqr) {
            // accelerate_steps = ceil(intersection_distance(initial_rate, final_rate, acceleration, block->steps));
            // intersection_distance(float initial_rate, float final_rate, float acceleration, float distance)
            // (2.0*acceleration*distance-initial_rate*initial_rate+final_rate*final_rate)/(4.0*acceleration);
            accelerate_steps = final_rate_sqr - initial_rate_sqr + acceleration_x4 - 1;
            if (block->steps & 1)
                accelerate_steps += acceleration_x2;
            accelerate_steps /= acceleration_x4;
            accelerate_steps += (block->steps >> 1);
            if (accelerate_steps > block->steps)
                accelerate_steps = block->steps;
        } else {
            decelerate_steps = initial_rate_sqr - final_rate_sqr;
            if (block->steps & 1)
                decelerate_steps += acceleration_x2;
            decelerate_steps /= acceleration_x4;
            decelerate_steps += (block->steps >> 1);
            if (decelerate_steps > block->steps)
                decelerate_steps = block->steps;
            accelerate_steps = block->steps - decelerate_steps;
        }
    }

    block->accelerate_until = accelerate_steps;
    block->decelerate_after = accelerate_steps + plateau_steps;
    block->initial_rate = initial_rate;
    block->final_rate = final_rate;
}

void PulseGen::Move(pos_t target, steps_t feed_rate) {
    // Prepare to set up new block
    block_t *block = &block_buffer[block_buffer_head];

    block->steps = abs(target - position);

    // Bail if this is a zero-length block
    if (block->steps <= dropsegments)
        return;

    // Direction and speed for this block
    block->direction = (target > position);
    block->nominal_rate = feed_rate;

    // Acceleration of the segment, in steps/sec^2
    block->acceleration = acceleration;
    block->acceleration_rate = block->acceleration * (rate_t)((float)F_CPU / (F_CPU / STEP_TIMER_DIVIDER));

    // Perform the trapezoid calculations
    CalculateTrapezoid(block, max_jerk, max_jerk);

    // TODO: Move the buffer head
    //block_buffer_head++;

    position = target;
}

st_timer_t PulseGen::Step(const MotorParams &motorParams) {
    if (!current_block) {
        // TODO: fetch next block
        if (!block_buffer_head)
            current_block = &block_buffer[block_buffer_head++];
        if (!current_block)
            return 0;

        // Set direction early so that the direction-change delay is accounted for
        TMC2130::SetDir(motorParams, current_block->direction);

        // Initializes the trapezoid generator from the current block.
        deceleration_time = 0;
        acc_step_rate = uint16_t(current_block->initial_rate);
        acceleration_time = calc_timer(acc_step_rate, step_loops);
        steps_completed = 0;

        // Set the nominal step loops to zero to indicate, that the timer value is not known yet.
        // That means, delay the initialization of nominal step rate and step loops until the steady
        // state is reached.
        step_loops_nominal = 0;
    }

    // Step the motor
    for (uint8_t i = 0; i < step_loops; ++i) {
        TMC2130::Step(motorParams);
        if (++steps_completed >= current_block->steps)
            break;
    }

    // Calculate new timer value
    // 13.38-14.63us for steady state,
    // 25.12us for acceleration / deceleration.
    st_timer_t timer;
    if (steps_completed <= current_block->accelerate_until) {
        // v = t * a   ->   acc_step_rate = acceleration_time * current_block->acceleration_rate
        acc_step_rate = mulU24X24toH16(acceleration_time, current_block->acceleration_rate);
        acc_step_rate += uint16_t(current_block->initial_rate);
        // upper limit
        if (acc_step_rate > uint16_t(current_block->nominal_rate))
            acc_step_rate = current_block->nominal_rate;
        // step_rate to timer interval
        timer = calc_timer(acc_step_rate, step_loops);
        acceleration_time += timer;
    } else if (steps_completed > current_block->decelerate_after) {
        st_timer_t step_rate = mulU24X24toH16(deceleration_time, current_block->acceleration_rate);

        if (step_rate > acc_step_rate) { // Check step_rate stays positive
            step_rate = uint16_t(current_block->final_rate);
        } else {
            step_rate = acc_step_rate - step_rate; // Decelerate from acceleration end point.

            // lower limit
            if (step_rate < current_block->final_rate)
                step_rate = uint16_t(current_block->final_rate);
        }

        // Step_rate to timer interval.
        timer = calc_timer(step_rate, step_loops);
        deceleration_time += timer;
    } else {
        if (!step_loops_nominal) {
            // Calculation of the steady state timer rate has been delayed to the 1st tick of the steady state to lower
            // the initial interrupt blocking.
            timer_nominal = calc_timer(uint16_t(current_block->nominal_rate), step_loops);
            step_loops_nominal = step_loops;
        }
        timer = timer_nominal;
    }

    // If current block is finished, reset pointer
    if (steps_completed >= current_block->steps) {
        current_block = nullptr;
    }

    return timer;
}

} // namespace motor
} // namespace modules
