/// @file pulley.h
#pragma once
#include "../config/config.h"
#include "axisunit.h"
#include "../unit.h"
#include "movable_base.h"

namespace modules {

/// The pulley namespace provides all necessary facilities related to the logical model of the pulley device of the MMU unit.
namespace pulley {

namespace mm = modules::motion;

/// The Pulley model is an analogy to Idler and Selector.
/// It encapsulates the same error handling principles like the other two (motored) modules.
/// On the other hand - the Pulley is much simpler, there is no homing, engage/disengage and slots,
/// but it supports free rotation in either directions and some computation on top of it.
class Pulley : public motion::MovableBase {
public:
    inline constexpr Pulley()
        : MovableBase(mm::Pulley) {}

    /// Performs one step of the state machine according to currently planned operation
    /// @returns true if the pulley is ready to accept new commands (i.e. it has finished the last operation)
    bool Step();

    void PlanMove(unit::U_mm delta, unit::U_mm_s feed_rate, unit::U_mm_s end_rate = { 0 });

    /// @returns rounded current position (rotation) of the Pulley
    /// This exists purely to avoid expensive float (long double) computations of distance traveled by the filament
    int32_t CurrentPosition_mm();

    void InitAxis();
    void Disable();

protected:
    virtual void PrepareMoveToPlannedSlot() override {}
    virtual void PlanHomingMove() override {}
    virtual void FinishHomingAndPlanMoveToParkPos() override;
    virtual void FinishMove() override {}
};

/// The one and only instance of Pulley in the FW
extern Pulley pulley;

} // namespace pulley
} // namespace modules

namespace mpu = modules::pulley;
