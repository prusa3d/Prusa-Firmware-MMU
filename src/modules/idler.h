#pragma once
#include <stdint.h>

/// Idler model
/// Handles asynchronnous Engaging / Disengaging operations
/// Keeps track of idler's current state

namespace modules {
namespace idler {

class Idler {
public:
    enum {
        Ready = 0,
        Moving,
        Failed
    };

    inline Idler()
        : state(Ready)
        , plannedEngage(false)
        , plannedSlot(0)
        , currentSlot(0)
        , currentlyEngaged(false) {}

    // public operations on the idler

    /// @retuns false in case an operation is already underway
    bool Engage(uint8_t slot);
    /// @retuns false in case an operation is already underway
    bool Disengage();
    /// @retuns false in case an operation is already underway
    bool Home();

    /// @returns true if the idler is ready to accept new commands (i.e. it has finished the last operation)
    bool Step();

    /// @returns the current state of idler - engaged / disengaged
    /// this state is updated only when a planned move is successfully finished, so it is safe for higher-level
    /// state machines to use this call as a waiting condition for the desired state of the idler
    inline bool Engaged() const { return currentlyEngaged; }
    inline uint8_t Slot() const { return currentSlot; }

private:
    constexpr static const uint16_t slotPositions[5] = { 1, 2, 3, 4, 5 }; // @@TODO

    /// internal state of the automaton
    uint8_t state;

    /// direction of travel - engage/disengage
    bool plannedEngage;
    uint8_t plannedSlot;

    /// current state
    uint8_t currentSlot;
    bool currentlyEngaged;
};

extern Idler idler;

} // namespace idler
} // namespace modules
