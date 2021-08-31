#pragma once
#include <stdint.h>
#include "../config/axis.h"
#include "../hal/tmc2130.h"

namespace modules {
namespace motion {

/// Base class for movable modules - #Idler and #Selector contains the common code
class MovableBase {
public:
    /// Internal states of the state machine
    enum {
        Ready = 0, // intentionally set as zero in order to allow zeroing the Idler structure upon startup -> avoid explicit initialization code
        Moving,
        Failed
    };

    /// Operation (Engage/Disengage/MoveToSlot) return values
    enum class OperationResult : uint8_t {
        Accepted, ///< the operation has been successfully started
        Refused, ///< another operation is currently underway, cannot start a new one
        Failed ///< the operation could not been started due to HW issues
    };

    inline constexpr MovableBase()
        : state(Ready)
        , plannedSlot(0)
        , currentSlot(0) {}

    /// virtual ~MovableBase(); intentionally disabled, see description in logic::CommandBase

    /// @returns currently active slot
    /// this state is updated only when a planned move is successfully finished, so it is safe for higher-level
    /// state machines to use this call as a waiting condition for the desired state of the derive class (idler/selector)
    inline uint8_t Slot() const { return currentSlot; }

    /// @returns internal state of the state machine
    inline uint8_t State() const { return state; }

    inline hal::tmc2130::ErrorFlags TMCErrorFlags() const { return tmcErrorFlags; }

protected:
    /// internal state of the automaton
    uint8_t state;

    /// planned slot - where to move to
    uint8_t plannedSlot;

    /// current slot
    uint8_t currentSlot;

    /// cached TMC2130 error flags - being read only if the axis is enabled and doing something (moving)
    hal::tmc2130::ErrorFlags tmcErrorFlags;

    virtual void PrepareMoveToPlannedSlot() = 0;

    OperationResult InitMovement(config::Axis axis);

    void PerformMove(config::Axis axis);
};

} // namespace motion
} // namespace modules
