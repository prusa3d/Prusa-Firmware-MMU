#pragma once
#include <stdint.h>

/// A complete set of progress codes which may be reported while running a high-level command/operation
/// This header file shall be included in the printer's firmware as well as a reference,
/// therefore the progress codes have been extracted to one place
enum class ProgressCode : uint_fast8_t {
    OK = 0, ///< finished ok

    EngagingIdler,
    DisengagingIdler,
    UnloadingToFinda,
    UnloadingToPulley,
    FeedingToFinda,
    FeedingToBondtech,
    AvoidingGrind,
    FinishingMoves,

    ERR1DisengagingIdler,
    ERR1EngagingIdler,
    ERR1WaitingForUser,
    ERRInternal,
    ERR1HelpingFilament,
    ERR1TMCInitFailed,

    UnloadingFilament,
    LoadingFilament,
    SelectingFilamentSlot,
    PreparingBlade,
    PushingFilament,
    PerformingCut,
    ReturningSelector,
    ParkingSelector,
    EjectingFilament,
};
