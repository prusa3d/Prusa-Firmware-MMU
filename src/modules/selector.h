#pragma once
#include <stdint.h>

namespace modules {

/// The selector namespace provides all necessary facilities related to the logical model of the selector device of the MMU unit.
namespace selector {

/// The selector model - handles asynchronnous move operations between filament individual slots and keeps track of selector's current state.
class Selector {
public:
    /// Internal states of selector's state machine
    enum {
        Ready = 0,
        Moving,
        Failed
    };

    inline constexpr Selector()
        : state(Ready)
        , plannedSlot(0)
        , currentSlot(0) {}

    /// Plan move of the selector to a specific filament slot
    /// @param slot index to move to
    /// @returns false in case an operation is already underway
    bool MoveToSlot(uint8_t slot);

    /// Plan homing of the selector's axis
    /// @returns false in case an operation is already underway
    bool Home();

    /// Performs one step of the state machine according to currently planned operation.
    /// @returns true if the selector is ready to accept new commands (i.e. it has finished the last operation)
    bool Step();

    /// @returns the current slot of selector
    /// this state is updated only when a planned move is successfully finished, so it is safe for higher-level
    /// state machines to use this call as a waiting condition for the desired state of the selector
    inline uint8_t Slot() const { return currentSlot; }

    /// @returns predefined positions of individual slots
    inline static uint16_t SlotPosition(uint8_t slot) { return slotPositions[slot]; }

private:
    /// slots 0-4 are the real ones, the 5th is the farthest parking positions
    static const uint16_t slotPositions[6];

    /// internal state of the automaton
    uint8_t state;
    uint8_t plannedSlot;

    /// current state
    uint8_t currentSlot;
};

/// The one and only instance of Selector in the FW
extern Selector selector;

} // namespace selector
} // namespace modules
