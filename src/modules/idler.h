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

    /// @returns currently active slot
    inline uint8_t Slot() const { return currentSlot; }

    /// @returns predefined positions of individual slots
    inline static uint16_t SlotPosition(uint8_t slot) { return slotPositions[slot]; }

private:
    /// slots 0-4 are the real ones, the 5th is the idle position
    static const uint16_t slotPositions[6];

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
