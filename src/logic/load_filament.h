/// @file load_filament.h
#pragma once
#include <stdint.h>
#include "command_base.h"
#include "feed_to_finda.h"
#include "retract_from_finda.h"

namespace logic {

/// @brief A high-level command state machine - handles the complex logic of loading filament into a filament slot.
class LoadFilament : public CommandBase {
public:
    inline LoadFilament()
        : CommandBase() {}

    /// Restart the automaton
    /// @param param index of filament slot to load
    void Reset(uint8_t param) override;

    /// Restart the automaton for unlimited rotation of the Pulley
    /// @param param index of filament slot to load
    void ResetUnlimited(uint8_t param);

    /// @returns true if the state machine finished its job, false otherwise
    bool StepInner() override;

private:
    void GoToRetractingFromFinda();
    void Reset2(bool feedPhaseLimited);

    /// Common code for a correct completion of UnloadFilament
    void FinishedCorrectly();

    FeedToFinda feed;
    RetractFromFinda retract;
};

/// The one and only instance of LoadFilament state machine in the FW
extern LoadFilament loadFilament;

} // namespace logic
