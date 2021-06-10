#pragma once
#include <stdint.h>

/// Selector model
/// Handles asynchronnous move operations between filament individual slots
/// Keeps track of selector's current state

namespace modules {
namespace selector {

class Selector {
public:
    enum {
        Ready = 0,
        Moving,
        Failed
    };

    inline Selector()
        : state(Ready)
        , plannedSlot(0)
        , currentSlot(0) {}

    // public operations on the selector

    /// @retuns false in case an operation is already underway
    bool MoveToSlot(uint8_t slot);
    /// @retuns false in case an operation is already underway
    bool Home();

    /// @returns true if the selector is ready to accept new commands (i.e. it has finished the last operation)
    bool Step();

    /// @returns the current slot of selector
    /// this state is updated only when a planned move is successfully finished, so it is safe for higher-level
    /// state machines to use this call as a waiting condition for the desired state of the selector
    inline uint8_t Slot() const { return currentSlot; }

private:
    constexpr static const uint16_t slotPositions[5] = { 1, 2, 3, 4, 5 }; // @@TODO

    /// internal state of the automaton
    uint8_t state;
    uint8_t plannedSlot;

    /// current state
    uint8_t currentSlot;
};

extern Selector selector;

} // namespace selector
} // namespace modules
