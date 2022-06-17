/// @file protocol.cpp
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
namespace protocol {

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

DecodeStatus Protocol::DecodeRequest(uint8_t c) {
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
        case 'W': // write is gonna be a special one
        case 'K':
        case 'F':
        case 'f':
        case 'H':
        case 'R':
            requestMsg.code = (RequestMsgCodes)c;
            requestMsg.value = 0;
            requestMsg.value2 = 0;
            rqState = (c == 'W') ? RequestStates::Address : RequestStates::Value; // prepare special automaton path for Write commands
            return DecodeStatus::NeedMoreData;
        default:
            requestMsg.code = RequestMsgCodes::unknown;
            rqState = RequestStates::Error;
            return DecodeStatus::Error;
        }
    case RequestStates::Value:
        if (IsHexDigit(c)) {
            requestMsg.value <<= 4U;
            requestMsg.value |= Char2Nibble(c);
            return DecodeStatus::NeedMoreData;
        } else if (IsNewLine(c)) {
            rqState = RequestStates::Code;
            return DecodeStatus::MessageCompleted;
        } else {
            requestMsg.code = RequestMsgCodes::unknown;
            rqState = RequestStates::Error;
            return DecodeStatus::Error;
        }
    case RequestStates::Address:
        if (IsHexDigit(c)) {
            requestMsg.value <<= 4U;
            requestMsg.value |= Char2Nibble(c);
            return DecodeStatus::NeedMoreData;
        } else if (c == ' ') { // end of address, value coming
            rqState = RequestStates::WriteValue;
            return DecodeStatus::NeedMoreData;
        } else {
            requestMsg.code = RequestMsgCodes::unknown;
            rqState = RequestStates::Error;
            return DecodeStatus::Error;
        }
    case RequestStates::WriteValue:
        if (IsHexDigit(c)) {
            requestMsg.value2 <<= 4U;
            requestMsg.value2 |= Char2Nibble(c);
            return DecodeStatus::NeedMoreData;
        } else if (IsNewLine(c)) {
            rqState = RequestStates::Code;
            return DecodeStatus::MessageCompleted;
        } else {
            requestMsg.code = RequestMsgCodes::unknown;
            rqState = RequestStates::Error;
            return DecodeStatus::Error;
        }
    default: //case error:
        if (IsNewLine(c)) {
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
    uint8_t i = 1;
    txbuff[0] = (uint8_t)msg.code;
    uint8_t v = msg.value >> 4;
    if (v != 0) { // skip the first '0' if any
        txbuff[i] = Nibble2Char(v);
        ++i;
    }
    v = msg.value & 0xf;
    txbuff[i] = Nibble2Char(v);
    ++i;
    txbuff[i] = '\n';
    ++i;
    return i;
    static_assert(4 <= MaxRequestSize(), "Request message length exceeded the maximum size, increase the magic constant in MaxRequestSize()");
}

uint8_t Protocol::EncodeWriteRequest(const RequestMsg &msg, uint16_t value2, uint8_t *txbuff) {
    uint8_t i = BeginEncodeRequest(msg, txbuff);
    // dump the value
    i += Value2Hex(value2, txbuff + i);

    txbuff[i] = '\n';
    ++i;
    return i;
}

DecodeStatus Protocol::DecodeResponse(uint8_t c) {
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
        case 'F':
        case 'f':
        case 'H':
        case 'R':
            responseMsg.request.code = (RequestMsgCodes)c;
            responseMsg.request.value = 0;
            rspState = ResponseStates::RequestValue;
            return DecodeStatus::NeedMoreData;
        case 0x0a:
        case 0x0d:
            // skip leading whitespace if any (makes integration with other SW easier/tolerant)
            return DecodeStatus::NeedMoreData;
        default:
            rspState = ResponseStates::Error;
            return DecodeStatus::Error;
        }
    case ResponseStates::RequestValue:
        if (IsHexDigit(c)) {
            responseMsg.request.value <<= 4U;
            responseMsg.request.value += Char2Nibble(c);
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
        case 'B':
            rspState = ResponseStates::ParamValue;
            responseMsg.paramCode = (ResponseMsgParamCodes)c;
            responseMsg.paramValue = 0;
            return DecodeStatus::NeedMoreData;
        default:
            responseMsg.paramCode = ResponseMsgParamCodes::unknown;
            rspState = ResponseStates::Error;
            return DecodeStatus::Error;
        }
    case ResponseStates::ParamValue:
        if (IsHexDigit(c)) {
            responseMsg.paramValue <<= 4U;
            responseMsg.paramValue += Char2Nibble(c);
            return DecodeStatus::NeedMoreData;
        } else if (IsNewLine(c)) {
            rspState = ResponseStates::RequestCode;
            return DecodeStatus::MessageCompleted;
        } else {
            responseMsg.paramCode = ResponseMsgParamCodes::unknown;
            rspState = ResponseStates::Error;
            return DecodeStatus::Error;
        }
    default: //case error:
        if (IsNewLine(c)) {
            rspState = ResponseStates::RequestCode;
            return DecodeStatus::MessageCompleted;
        } else {
            responseMsg.paramCode = ResponseMsgParamCodes::unknown;
            return DecodeStatus::Error;
        }
    }
}

uint8_t Protocol::EncodeResponseCmdAR(const RequestMsg &msg, ResponseMsgParamCodes ar, uint8_t *txbuff) {
    uint8_t i = BeginEncodeRequest(msg, txbuff);
    txbuff[i] = (uint8_t)ar;
    ++i;
    txbuff[i] = '\n';
    ++i;
    return i;
}

uint8_t Protocol::EncodeResponseReadFINDA(const RequestMsg &msg, uint8_t findaValue, uint8_t *txbuff) {
    //    txbuff[0] = (uint8_t)msg.code;
    //    txbuff[1] = msg.value + '0';
    //    txbuff[2] = ' ';
    //    txbuff[3] = (uint8_t)ResponseMsgParamCodes::Accepted;
    //    txbuff[4] = findaValue + '0';
    //    txbuff[5] = '\n';
    //    return 6;
    return EncodeResponseRead(msg, true, findaValue, txbuff);
}

uint8_t Protocol::EncodeResponseVersion(const RequestMsg &msg, uint16_t value, uint8_t *txbuff) {
    //    txbuff[0] = (uint8_t)msg.code;
    //    txbuff[1] = msg.value + '0';
    //    txbuff[2] = ' ';
    //    txbuff[3] = (uint8_t)ResponseMsgParamCodes::Accepted;
    //    uint8_t *dst = txbuff + 4;
    //    dst += Value2Hex(value, dst);
    //    *dst = '\n';
    //    return dst - txbuff + 1;
    return EncodeResponseRead(msg, true, value, txbuff);
}

uint8_t Protocol::EncodeResponseQueryOperation(const RequestMsg &msg, ResponseCommandStatus rcs, uint8_t *txbuff) {
    txbuff[0] = (uint8_t)msg.code;
    txbuff[1] = msg.value + '0';
    txbuff[2] = ' ';
    txbuff[3] = (uint8_t)rcs.code;
    uint8_t *dst = txbuff + 4;
    if (rcs.code != ResponseMsgParamCodes::Finished) {
        dst += Value2Hex(rcs.value, dst);
    }
    *dst = '\n';
    return dst - txbuff + 1;
}

uint8_t Protocol::EncodeResponseRead(const RequestMsg &msg, bool accepted, uint16_t value2, uint8_t *txbuff) {
    uint8_t i = BeginEncodeRequest(msg, txbuff);
    if (accepted) {
        txbuff[i] = (uint8_t)ResponseMsgParamCodes::Accepted;
        ++i;
        // dump the value
        i += Value2Hex(value2, txbuff + i);
    } else {
        txbuff[i] = (uint8_t)ResponseMsgParamCodes::Rejected;
        ++i;
    }
    txbuff[i] = '\n';
    ++i;
    return i;
}

uint8_t Protocol::Value2Hex(uint16_t value, uint8_t *dst) {
    constexpr uint16_t topNibbleMask = 0xf000;
    if (value == 0) {
        *dst = '0';
        return 1;
    }
    // skip initial zeros
    uint8_t charsOut = 4;
    while ((value & topNibbleMask) == 0) {
        value <<= 4U;
        --charsOut;
    }
    for (uint8_t i = 0; i < charsOut; ++i) {
        uint8_t n = (value & topNibbleMask) >> (8U + 4U);
        value <<= 4U;
        *dst = Nibble2Char(n);
        ++dst;
    }
    return charsOut;
}

uint8_t Protocol::BeginEncodeRequest(const RequestMsg &msg, uint8_t *txbuff) {
    uint8_t i = 1;
    txbuff[0] = (uint8_t)msg.code;
    uint8_t v = msg.value >> 4U;
    if (v != 0) { // skip the first '0' if any
        txbuff[i] = Nibble2Char(v);
        ++i;
    }
    v = msg.value & 0xfU;
    txbuff[i] = Nibble2Char(v);
    ++i;
    txbuff[i] = ' ';
    return i + 1;
}

} // namespace protocol
} // namespace modules
