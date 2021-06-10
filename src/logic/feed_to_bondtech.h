#pragma once
#include <stdint.h>

/// @brief Feed filament to Bondtech gears of the printer
///
/// Continuously feed filament until the printer detects the filament in its filament sensor

namespace logic {

struct FeedToBondtech {
    enum {
        EngagingIdler,
        PushingFilament,
        UnloadBackToPTFE,
        DisengagingIdler,
        OK,
        Failed
    };

    inline FeedToBondtech()
        : state(OK)
        , maxRetries(1) {}

    /// Restart the automaton
    void Reset(uint8_t maxRetries);

    /// @returns true if the state machine finished its job, false otherwise
    bool Step();

    /// This method may be used to check the result of the automaton
    /// @returns OK if everything went OK and printer's filament sensor triggered
    /// @returns Failed if the maximum feed length has been reached without the the printer's fsensor trigger being reported
    inline uint8_t State() const { return state; }

private:
    uint8_t state;
    uint8_t maxRetries;
};

} // namespace logic
