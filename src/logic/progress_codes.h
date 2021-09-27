/// @file progress_codes.h
#pragma once
#include <stdint.h>

/// A complete set of progress codes which may be reported while running a high-level command/operation
/// This header file shall be included in the printer's firmware as well as a reference,
/// therefore the progress codes have been extracted to one place
enum class ProgressCode : uint_fast8_t {
    OK = 0, ///< finished ok

    EngagingIdler, // P1
    DisengagingIdler, // P2
    UnloadingToFinda, // P3
    UnloadingToPulley, //P4
    FeedingToFinda, // P5
    FeedingToBondtech, // P6
    AvoidingGrind, // P7
    FinishingMoves, // P8

    ERRDisengagingIdler, // P9
    ERREngagingIdler, // P10
    ERRWaitingForUser, // P11
    ERRInternal, // P12
    ERRHelpingFilament, // P13
    ERRTMCFailed, // P14

    UnloadingFilament, // P15
    LoadingFilament, // P16
    SelectingFilamentSlot, // P17
    PreparingBlade, // P18
    PushingFilament, // P19
    PerformingCut, // P20
    ReturningSelector, // P21
    ParkingSelector, // P22
    EjectingFilament, // P23
    RetractingFromFinda, // P24
};
