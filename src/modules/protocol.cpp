#include "protocol.h"

// protocol definition
// command: Q0
// meaning: query operation status
// Query/command: query
// Expected reply from the MMU:
//  any of the running operation statuses: OID: [T|L|U|E|C|W|K][0-4]
//  <OID> P[0-9]      : command being processed i.e. operation running, may contain a state number
//  <OID> E[0-9][0-9] : error 1-9 while doing a tool change
//  <OID> F           : operation finished - will be repeated to "Q" messages until a new command is issued

namespace modules {

// decoding automaton
// states:  input -> transition into state
// Code      QTLMUXPSBEWK -> msgcode
//           \n ->start
//           * ->error
// error     \n ->start
//           * ->error
// msgcode   0-9 ->msgvalue
//           * ->error
// msgvalue  0-9 ->msgvalue
//           \n ->start successfully accepted command

Protocol::DecodeStatus Protocol::DecodeRequest(uint8_t c) {
    switch (rqState) {
    case RequestStates::Code:
        switch (c) {
        case 'Q':
        case 'T':
        case 'L':
        case 'M':
        case 'U':
        case 'X':
        case 'P':
        case 'S':
        case 'B':
        case 'E':
        case 'W':
        case 'K':
            requestMsg.code = (RequestMsgCodes)c;
            requestMsg.value = 0;
            rqState = RequestStates::Value;
            return DecodeStatus::NeedMoreData;
        default:
            requestMsg.code = RequestMsgCodes::unknown;
            rqState = RequestStates::Error;
            return DecodeStatus::Error;
        }
    case RequestStates::Value:
        if (c >= '0' && c <= '9') {
            requestMsg.value *= 10;
            requestMsg.value += c - '0';
            return DecodeStatus::NeedMoreData;
        } else if (c == '\n') {
            rqState = RequestStates::Code;
            return DecodeStatus::MessageCompleted;
        } else {
            requestMsg.code = RequestMsgCodes::unknown;
            rqState = RequestStates::Error;
            return DecodeStatus::Error;
        }
    default: //case error:
        if (c == '\n') {
            rqState = RequestStates::Code;
            return DecodeStatus::MessageCompleted;
        } else {
            requestMsg.code = RequestMsgCodes::unknown;
            rqState = RequestStates::Error;
            return DecodeStatus::Error;
        }
    }
}

uint8_t Protocol::EncodeRequest(const RequestMsg &msg, uint8_t *txbuff) {
    txbuff[0] = (uint8_t)msg.code;
    txbuff[1] = msg.value + '0';
    txbuff[2] = '\n';
    return 3;
}

Protocol::DecodeStatus Protocol::DecodeResponse(uint8_t c) {
    switch (rspState) {
    case ResponseStates::RequestCode:
        switch (c) {
        case 'Q':
        case 'T':
        case 'L':
        case 'M':
        case 'U':
        case 'X':
        case 'P':
        case 'S':
        case 'B':
        case 'E':
        case 'W':
        case 'K':
            responseMsg.request.code = (RequestMsgCodes)c;
            responseMsg.request.value = 0;
            rspState = ResponseStates::RequestValue;
            return DecodeStatus::NeedMoreData;
        default:
            rspState = ResponseStates::Error;
            return DecodeStatus::Error;
        }
    case ResponseStates::RequestValue:
        if (c >= '0' && c <= '9') {
            responseMsg.request.value *= 10;
            responseMsg.request.value += c - '0';
            return DecodeStatus::NeedMoreData;
        } else if (c == ' ') {
            rspState = ResponseStates::ParamCode;
            return DecodeStatus::NeedMoreData;
        } else {
            rspState = ResponseStates::Error;
            return DecodeStatus::Error;
        }
    case ResponseStates::ParamCode:
        switch (c) {
        case 'P':
        case 'E':
        case 'F':
        case 'A':
        case 'R':
            rspState = ResponseStates::ParamValue;
            responseMsg.params.code = (RequestMsgCodes)c; // @@TODO this is not clean
            responseMsg.params.value = 0;
            return DecodeStatus::NeedMoreData;
        default:
            rspState = ResponseStates::Error;
            return DecodeStatus::Error;
        }
    case ResponseStates::ParamValue:
        if (c >= '0' && c <= '9') {
            responseMsg.params.value *= 10;
            responseMsg.params.value += c - '0';
            return DecodeStatus::NeedMoreData;
        } else if (c == '\n') {
            rspState = ResponseStates::RequestCode;
            return DecodeStatus::MessageCompleted;
        } else {
            rspState = ResponseStates::Error;
            return DecodeStatus::Error;
        }
    default: //case error:
        if (c == '\n') {
            rspState = ResponseStates::RequestCode;
            return DecodeStatus::MessageCompleted;
        } else {
            return DecodeStatus::Error;
        }
    }
}

uint8_t Protocol::EncodeResponseCmdAR(const RequestMsg &msg, ResponseMsgParamCodes ar, uint8_t *txbuff) {
    txbuff[0] = (uint8_t)msg.code;
    txbuff[1] = msg.value + '0';
    txbuff[2] = ' ';
    txbuff[3] = (uint8_t)ar;
    txbuff[4] = '\n';
    return 5;
}

uint8_t Protocol::EncodeResponseReadFINDA(const RequestMsg &msg, uint8_t findaValue, uint8_t *txbuff) {
    txbuff[0] = (uint8_t)msg.code;
    txbuff[1] = msg.value + '0';
    txbuff[2] = ' ';
    txbuff[3] = (uint8_t)ResponseMsgParamCodes::Accepted;
    txbuff[4] = findaValue + '0';
    txbuff[5] = '\n';
    return 6;
}

uint8_t Protocol::EncodeResponseVersion(const RequestMsg &msg, uint8_t value, uint8_t *txbuff) {
    txbuff[0] = (uint8_t)msg.code;
    txbuff[1] = msg.value + '0';
    txbuff[2] = ' ';
    txbuff[3] = (uint8_t)ResponseMsgParamCodes::Accepted;
    uint8_t *dst = txbuff + 4;
    if (value < 10) {
        *dst++ = value + '0';
    } else if (value < 100) {
        *dst++ = value / 10 + '0';
        *dst++ = value % 10 + '0';
    } else {
        *dst++ = value / 100 + '0';
        *dst++ = (value / 10) % 10 + '0';
        *dst++ = value % 10 + '0';
    }
    *dst = '\n';
    return dst - txbuff + 1;
}

uint8_t Protocol::EncodeResponseQueryOperation(const RequestMsg &msg, ResponseMsgParamCodes code, uint8_t value, uint8_t *txbuff) {
    txbuff[0] = (uint8_t)msg.code;
    txbuff[1] = msg.value + '0';
    txbuff[2] = ' ';
    txbuff[3] = (uint8_t)code;
    uint8_t *dst = txbuff + 4;
    if (code != ResponseMsgParamCodes::Finished) {
        if (value < 10) {
            *dst++ = value + '0';
        } else if (value < 100) {
            *dst++ = value / 10 + '0';
            *dst++ = value % 10 + '0';
        } else {
            *dst++ = value / 100 + '0';
            *dst++ = (value / 10) % 10 + '0';
            *dst++ = value % 10 + '0';
        }
    }
    *dst = '\n';
    return dst - txbuff + 1;
}

} // namespace modules
