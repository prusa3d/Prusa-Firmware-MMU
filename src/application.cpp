/// @file
#include "application.h"

#include "modules/leds.h"
#include "modules/globals.h"
#include "modules/user_input.h"
#include "modules/finda.h"
#include "modules/fsensor.h"
#include "modules/serial.h"

#include "logic/command_base.h"
#include "logic/cut_filament.h"
#include "logic/eject_filament.h"
#include "logic/home.h"
#include "logic/load_filament.h"
#include "logic/move_selector.h"
#include "logic/no_command.h"
#include "logic/set_mode.h"
#include "logic/tool_change.h"
#include "logic/unload_filament.h"

#include "version.h"

#include "panic.h"

/// Global instance of the protocol codec
static mp::Protocol protocol;

Application application;

Application::Application()
    : lastCommandProcessedMs(0)
    , currentCommand(&logic::noCommand)
    , currentCommandRq(mp::RequestMsgCodes::Reset, 0) {}

void Application::CheckManualOperation() {
    uint16_t ms = mt::timebase.Millis();
    constexpr uint16_t idleDelay = 1000U;
    if (ms - lastCommandProcessedMs < idleDelay) {
        if (currentCommand->State() == ProgressCode::OK) {
            mui::userInput.Clear(); // consume bogus UI events while no command in progress and not in idle state yet
        }
        return;
    }

    lastCommandProcessedMs = ms - idleDelay; // prevent future overflows

    if (currentCommand->State() == ProgressCode::OK && mg::globals.FilamentLoaded() <= mg::FilamentLoadState::AtPulley) {
        if (mui::userInput.AnyEvent()) {
            switch (mui::userInput.ConsumeEvent()) {
            case mui::Event::Left:
                // move selector left if possible
                if (mg::globals.ActiveSlot() > 0) {
                    logic::moveSelector.Reset(mg::globals.ActiveSlot() - 1);
                    currentCommand = &logic::moveSelector;
                }
                break;
            case mui::Event::Middle:
                // plan load
                if (mg::globals.ActiveSlot() < config::toolCount) { // do we have a meaningful selector position?
                    logic::loadFilament.Reset(mg::globals.ActiveSlot());
                    currentCommand = &logic::loadFilament;
                }
                break;
            case mui::Event::Right:
                // move selector right if possible (including the park position)
                if (mg::globals.ActiveSlot() < config::toolCount) {
                    logic::moveSelector.Reset(mg::globals.ActiveSlot() + 1); // we allow also the park position
                    currentCommand = &logic::moveSelector;
                }
                break;
            default:
                break;
            }
        }
    }
}

mp::ResponseCommandStatus Application::RunningCommandStatus() const {
    switch (currentCommand->Error()) {
    case ErrorCode::RUNNING:
        return mp::ResponseCommandStatus(mp::ResponseMsgParamCodes::Processing, (uint16_t)currentCommand->State());
    case ErrorCode::OK:
        return mp::ResponseCommandStatus(mp::ResponseMsgParamCodes::Finished, 0);
    default:
        return mp::ResponseCommandStatus(mp::ResponseMsgParamCodes::Error, (uint16_t)currentCommand->Error());
    }
}

static constexpr const uint8_t maxMsgLen = 10;

void Application::ReportCommandAccepted(const mp::RequestMsg &rq, mp::ResponseMsgParamCodes status) {
    uint8_t tmp[maxMsgLen];
    uint8_t len = protocol.EncodeResponseCmdAR(rq, status, tmp);
    modules::serial::WriteToUSART(tmp, len);
}

void Application::PlanCommand(const modules::protocol::RequestMsg &rq) {
    if (currentCommand->State() == ProgressCode::OK) {
        // We are allowed to start a new command as the previous one is in the OK finished state
        // The previous command may be in an error state, but as long as it is in ProgressCode::OK (aka finished)
        // we are safe here. It is the responsibility of the printer to ask for a command error code
        // before issuing another one - if needed.
        switch (rq.code) {
        case mp::RequestMsgCodes::Cut:
            currentCommand = &logic::cutFilament;
            break;
        case mp::RequestMsgCodes::Eject:
            currentCommand = &logic::ejectFilament;
            break;
        case mp::RequestMsgCodes::Home:
            currentCommand = &logic::home;
            break;
        case mp::RequestMsgCodes::Load:
            currentCommand = &logic::loadFilament;
            break;
        case mp::RequestMsgCodes::Tool:
            currentCommand = &logic::toolChange;
            break;
        case mp::RequestMsgCodes::Unload:
            currentCommand = &logic::unloadFilament;
            break;
        case mp::RequestMsgCodes::Mode:
            currentCommand = &logic::setMode;
            break;
        default:
            currentCommand = &logic::noCommand;
            break;
        }
        currentCommandRq = rq; // save the Current Command Request for indentification of responses
        currentCommand->Reset(rq.value);
        ReportCommandAccepted(rq, mp::ResponseMsgParamCodes::Accepted);
    } else {
        ReportCommandAccepted(rq, mp::ResponseMsgParamCodes::Rejected);
    }
}

void Application::ReportFINDA(const mp::RequestMsg &rq) {
#ifdef DEBUG_FINDA
    using namespace hal;
    hu::usart1.puts("FINDA:");
    if (hal::gpio::ReadPin(FINDA_PIN) == hal::gpio::Level::high) {
        hu::usart1.puts(" TIRGGERED\n");
    } else {
        hu::usart1.puts(" NOT TRIGGERED\n");
    }
#endif //DEBUG_FINDA
    uint8_t rsp[maxMsgLen];
    uint8_t len = protocol.EncodeResponseReadFINDA(rq, mf::finda.Pressed(), rsp);
    modules::serial::WriteToUSART(rsp, len);
}

void Application::ReportVersion(const mp::RequestMsg &rq) {
    uint8_t v = 0;

    switch (rq.value) {
    case 0:
        v = project_version_major;
        break;
    case 1:
        v = project_version_minor;
        break;
    case 2:
        v = project_version_revision;
        break;
    case 3:
        // @@TODO may be allow reporting uint16_t number of errors,
        // but anything beyond 255 errors means there is something seriously wrong with the MMU
        v = mg::globals.DriveErrors();
        break;
    default:
        v = 0;
        break;
    }

    uint8_t rsp[10];
    uint8_t len = protocol.EncodeResponseVersion(rq, v, rsp);
    modules::serial::WriteToUSART(rsp, len);
}

void Application::ReportRunningCommand() {
    uint8_t rsp[maxMsgLen];
    uint8_t len = protocol.EncodeResponseQueryOperation(currentCommandRq, RunningCommandStatus(), rsp);
    modules::serial::WriteToUSART(rsp, len);
}

void Application::ProcessRequestMsg(const mp::RequestMsg &rq) {
    switch (rq.code) {
    case mp::RequestMsgCodes::Button:
        // behave just like if the user pressed a button
        mui::userInput.ProcessMessage(rq.value);
        break;
    case mp::RequestMsgCodes::Finda:
        // immediately report FINDA status
        ReportFINDA(rq);
        break;
    case mp::RequestMsgCodes::Query:
        // immediately report progress of currently running command
        ReportRunningCommand();
        break;
    case mp::RequestMsgCodes::Reset:
        // immediately reset the board - there is no response in this case
        hal::cpu::Reset();
        break;
    case mp::RequestMsgCodes::Version:
        ReportVersion(rq);
        break;
    case mp::RequestMsgCodes::Wait:
        break; // @@TODO - not used anywhere yet
    case mp::RequestMsgCodes::Cut:
    case mp::RequestMsgCodes::Eject:
    case mp::RequestMsgCodes::Home:
    case mp::RequestMsgCodes::Load:
    case mp::RequestMsgCodes::Tool:
    case mp::RequestMsgCodes::Unload:
        PlanCommand(rq);
        break;
    case mp::RequestMsgCodes::FilamentSensor: // set filament sensor state in the printer
        mfs::fsensor.ProcessMessage(rq.value != 0);
        ReportCommandAccepted(rq, mp::ResponseMsgParamCodes::Accepted);
        break;
    default:
        // respond with an error message
        break;
    }
}

bool Application::CheckMsgs() {
    using mpd = mp::DecodeStatus;
    while (modules::serial::Available()) {
        switch (protocol.DecodeRequest(modules::serial::ConsumeByte())) {
        case mpd::MessageCompleted:
            // process the input message
            return true;
            break;
        case mpd::NeedMoreData:
            // just continue reading
            break;
        case mpd::Error:
            // @@TODO what shall we do? Start some watchdog? We cannot send anything spontaneously
            break;
        }
    }
    return false;
}

void Application::Panic(ErrorCode ec) {
    currentCommand->Panic(ec);
}

void Application::Step() {
    CheckManualOperation();

    if (CheckMsgs()) {
        ProcessRequestMsg(protocol.GetRequestMsg());
    }

    currentCommand->Step();
}
