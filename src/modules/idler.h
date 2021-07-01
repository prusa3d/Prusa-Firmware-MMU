#pragma once
#include "../config/config.h"
#include <stdint.h>

namespace modules {

/// The idler namespace provides all necessary facilities related to the logical model of the idler device of the MMU unit.
namespace idler {

/// The Idler model handles asynchronnous Engaging / Disengaging operations and keeps track of idler's current state.
class Idler {
public:
    /// Internal states of idler's state machine
    enum {
        Ready = 0, // intentionally set as zero in order to allow zeroing the Idler structure upon startup -> avoid explicit initialization code
        Moving,
        Failed
    };

    inline constexpr Idler()
        : state(Ready)
        , plannedEngage(false)
        , plannedSlot(0)
        , currentSlot(0)
        , currentlyEngaged(false) {}

    /// Plan engaging of the idler to a specific filament slot
    /// @param slot index to be activated
    /// @returns false in case an operation is already underway
    bool Engage(uint8_t slot);

    /// Plan disengaging of the idler, i.e. parking the idler
    /// @returns false in case an operation is already underway
    bool Disengage();

    /// Plan homing of the idler axis
    /// @returns false in case an operation is already underway
    bool Home();

    /// Performs one step of the state machine according to currently planned operation
    /// @returns true if the idler is ready to accept new commands (i.e. it has finished the last operation)
    bool Step();

    /// @returns the current state of idler - engaged / disengaged
    /// this state is updated only when a planned move is successfully finished, so it is safe for higher-level
    /// state machines to use this call as a waiting condition for the desired state of the idler
    inline bool Engaged() const { return currentlyEngaged; }

    /// @returns currently active slot
    inline uint8_t Slot() const { return currentSlot; }

    /// @returns predefined positions of individual slots
    inline static uint16_t SlotPosition(uint8_t slot) { return config::idlerSlotPositions[slot]; }

    /// @returns the index of idle position of the idler, usually 5 in case of 0-4 valid indices of filament slots
    inline static constexpr uint8_t IdleSlotIndex() { return config::toolCount; }

private:
    /// internal state of the automaton
    uint8_t state;

    /// direction of travel - engage/disengage
    bool plannedEngage;
    uint8_t plannedSlot;

    /// current state
    uint8_t currentSlot;
    bool currentlyEngaged;
};

/// The one and only instance of Idler in the FW
extern Idler idler;

} // namespace idler
} // namespace modules
