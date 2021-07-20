#pragma once
#include <stdint.h>
#include "command_base.h"
#include "feed_to_finda.h"
#include "feed_to_bondtech.h"

namespace logic {

/// @brief A high-level command state machine - handles the complex logic of loading filament into a filament slot.
class LoadFilament : public CommandBase {
public:
    inline LoadFilament()
        : CommandBase() {}

    /// Restart the automaton
    /// @param param index of filament slot to load
    void Reset(uint8_t param) override;

    /// @returns true if the state machine finished its job, false otherwise
    bool StepInner() override;

private:
    FeedToFinda feed;
    FeedToBondtech james; // bond ;)
};

/// The one and only instance of LoadFilament state machine in the FW
extern LoadFilament loadFilament;

} // namespace logic
