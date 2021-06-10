#pragma once
#include <stdint.h>

/// A complete set of progress codes which may be reported while running a high-level command/operation
/// This header file shall be included in the printer's firmware as well as a reference,
/// therefore the progress codes have been extracted to one place

enum class ProgressCode : uint_fast8_t {
    OK = 0, ///< finished ok

    /// Unload Filament related progress codes
    EngagingIdler,
    UnloadingToFinda,
    DisengagingIdler,
    AvoidingGrind,
    FinishingMoves,
    ERR1DisengagingIdler,
    ERR1WaitingForUser,

    UnloadingFilament,
    SelectingFilamentSlot,
    FeedingToFINDA,
    PreparingBlade,
    PushingFilament,
    PerformingCut,
    ReturningSelector,
    ParkingSelector,
    EjectingFilament,
};
