#pragma once

#include <stdint.h>

/// MMU protocol implementation
/// See description of the new protocol in the MMU 2021 doc
/// @@TODO possibly add some checksum to verify the correctness of messages

namespace modules {

enum class RequestMsgCodes : uint8_t {
    unknown = 0,
    Query = 'Q',
    Tool = 'T',
    Load = 'L',
    Mode = 'M',
    Unload = 'U',
    Reset = 'X',
    Finda = 'P',
    Version = 'S',
    Button = 'B',
    Eject = 'E',
    Wait = 'W',
    Cut = 'K'
};

enum class ResponseMsgParamCodes : uint8_t {
    unknown = 0,
    Processing = 'P',
    Error = 'E',
    Finished = 'F',
    Accepted = 'A',
    Rejected = 'R'
};

/// A generic request message
struct Msg {
    RequestMsgCodes code;
    uint8_t value;
    inline Msg(RequestMsgCodes code, uint8_t value)
        : code(code)
        , value(value) {}
};

/// A request message
/// Requests are being sent by the printer into the MMU
/// It is the same structure as the generic Msg
using RequestMsg = Msg;

/// A response message
/// Responses are being sent from the MMU into the printer as a response to a request message
struct ResponseMsg {
    RequestMsg request; ///< response is always preceeded by the request message
    Msg params; ///< parameters of reply
    inline ResponseMsg(RequestMsg request, ResponseMsgParamCodes paramCode, uint8_t paramValue)
        : request(request)
        , params((RequestMsgCodes)paramCode, paramValue) {}
};

/// Protocol class is responsible for creating/decoding messages in Rx/Tx buffer
/// Beware - in the decoding more, it is meant to be a statefull instance which works through public methods
/// processing one input byte per call
class Protocol {
public:
    /// Message decoding return value
    enum class DecodeStatus : uint8_t {
        MessageCompleted, ///< message completed and successfully lexed
        NeedMoreData, ///< message incomplete yet, waiting for another byte to come
        Error, ///< input character broke message decoding
    };

    inline Protocol()
        : rqState(RequestStates::Code)
        , requestMsg(RequestMsgCodes::unknown, 0)
        , rspState(ResponseStates::RequestCode)
        , responseMsg(RequestMsg(RequestMsgCodes::unknown, 0), ResponseMsgParamCodes::unknown, 0) {
    }

    /// Takes the input byte c and steps one step through the state machine
    /// @returns state of the message being decoded
    DecodeStatus DecodeRequest(uint8_t c);

    /// Decodes response message in rxbuff
    /// @returns decoded response message structure
    DecodeStatus DecodeResponse(uint8_t c);

    /// Encodes request message msg into txbuff memory
    /// It is expected the txbuff is large enough to fit the message
    /// @returns number of bytes written into txbuff
    static uint8_t EncodeRequest(const RequestMsg &msg, uint8_t *txbuff);

    /// Encode generic response Command Accepted or Rejected
    /// @param msg source request message for this response
    /// @returns number of bytes written into txbuff
    static uint8_t EncodeResponseCmdAR(const RequestMsg &msg, ResponseMsgParamCodes ar, uint8_t *txbuff);

    /// Encode response to Read FINDA query
    /// @param msg source request message for this response
    /// @param findaValue 1/0 (on/off) status of FINDA
    /// @returns number of bytes written into txbuff
    static uint8_t EncodeResponseReadFINDA(const RequestMsg &msg, uint8_t findaValue, uint8_t *txbuff);

    /// Encode response to Version query
    /// @param msg source request message for this response
    /// @param value version number (0-255)
    /// @returns number of bytes written into txbuff
    static uint8_t EncodeResponseVersion(const RequestMsg &msg, uint8_t value, uint8_t *txbuff);

    /// Encode response to Query operation status
    /// @param msg source request message for this response
    /// @param code status of operation (Processing, Error, Finished)
    /// @param value related to status of operation(e.g. error code or progress)
    /// @returns number of bytes written into txbuff
    static uint8_t EncodeResponseQueryOperation(const RequestMsg &msg, ResponseMsgParamCodes code, uint8_t value, uint8_t *txbuff);

    /// @returns the most recently lexed request message
    inline const RequestMsg GetRequestMsg() const { return requestMsg; }

    /// @returns the most recently lexed response message
    inline const ResponseMsg GetResponseMsg() const { return responseMsg; }

private:
    enum class RequestStates : uint8_t {
        Code, ///< starting state - expects message code
        Value, ///< expecting code value
        Error ///< automaton in error state
    };

    RequestStates rqState;
    RequestMsg requestMsg;

    enum class ResponseStates : uint8_t {
        RequestCode, ///< starting state - expects message code
        RequestValue, ///< expecting code value
        ParamCode, ///< expecting param code
        ParamValue, ///< expecting param value
        Error ///< automaton in error state
    };

    ResponseStates rspState;
    ResponseMsg responseMsg;
};

} // namespace modules
