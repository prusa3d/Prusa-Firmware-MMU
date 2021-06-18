#pragma once
#include <stdint.h>
#include "command_base.h"
#include "unload_filament.h"
#include "feed_to_finda.h"

namespace logic {

/// A high-level command state machine
/// Handles the complex logic of cutting filament
class CutFilament : public CommandBase {
public:
    inline CutFilament()
        : CommandBase() {}

    /// Restart the automaton
    void Reset(uint8_t param) override;

    /// @returns true if the state machine finished its job, false otherwise
    bool Step() override;

    ProgressCode State() const override;

    ErrorCode Error() const override;

private:
    constexpr static const uint16_t cutStepsPre = 700;
    constexpr static const uint16_t cutStepsPost = 150;
    UnloadFilament unl; ///< a high-level command/operation may be used as a building block of other operations as well
    FeedToFinda feed;

    void SelectFilamentSlot();
};

extern CutFilament cutFilament;

} // namespace logic
