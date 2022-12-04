/// @file no_command.cpp
#include "no_command.h"
#include "../modules/buttons.h"
#include "../modules/finda.h"
#include "../modules/user_input.h"

namespace logic {

NoCommand noCommand;

bool NoCommand::StepInner() {
    switch (state) {
    case ProgressCode::OK:
        return true;
    case ProgressCode::ERRWaitingForUser: {
        // waiting for user buttons and/or a command from the printer
        mui::Event ev = mui::userInput.ConsumeEvent();
        switch (ev) {
        case mui::Event::Middle:
            switch (error) {
            case ErrorCode::FINDA_VS_EEPROM_DISREPANCY:
                // Retry
                if (!mf::finda.CheckFINDAvsEEPROM()) {
                    error = ErrorCode::FINDA_VS_EEPROM_DISREPANCY;
                    state = ProgressCode::ERRWaitingForUser;
                } else {
                    error = ErrorCode::OK;
                    state = ProgressCode::OK;
                }
                break;
            default:
                break;
            }
            break; // mui::Event::Middle
        default:
            break;
        }
        break; // ProgressCode::ERRWaitingForUser
    }
    default:
        // Do nothing
        break;
    }
    return false;
}
} // namespace logic
