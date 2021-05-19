#include "catch2/catch.hpp"
#include "protocol.h"

using Catch::Matchers::Equals;

TEST_CASE("protocol::EncodeRequests", "[protocol]") {
    using namespace modules;

    RequestMsgCodes code;
    uint8_t value;
    std::tie(code, value) = GENERATE(
        std::make_tuple(RequestMsgCodes::Button, 0),
        std::make_tuple(RequestMsgCodes::Button, 1),
        std::make_tuple(RequestMsgCodes::Button, 2),
        std::make_tuple(RequestMsgCodes::Cut, 0),
        std::make_tuple(RequestMsgCodes::Eject, 0),
        std::make_tuple(RequestMsgCodes::Finda, 0),
        std::make_tuple(RequestMsgCodes::Load, 0),
        std::make_tuple(RequestMsgCodes::Load, 1),
        std::make_tuple(RequestMsgCodes::Load, 2),
        std::make_tuple(RequestMsgCodes::Load, 3),
        std::make_tuple(RequestMsgCodes::Load, 4),
        std::make_tuple(RequestMsgCodes::Mode, 0),
        std::make_tuple(RequestMsgCodes::Mode, 1),
        std::make_tuple(RequestMsgCodes::Query, 0),
        std::make_tuple(RequestMsgCodes::Reset, 0),
        std::make_tuple(RequestMsgCodes::Tool, 0),
        std::make_tuple(RequestMsgCodes::Tool, 1),
        std::make_tuple(RequestMsgCodes::Tool, 2),
        std::make_tuple(RequestMsgCodes::Tool, 3),
        std::make_tuple(RequestMsgCodes::Tool, 4),
        std::make_tuple(RequestMsgCodes::Unload, 0),
        std::make_tuple(RequestMsgCodes::Version, 0),
        std::make_tuple(RequestMsgCodes::Version, 1),
        std::make_tuple(RequestMsgCodes::Version, 2),
        std::make_tuple(RequestMsgCodes::Wait, 0),
        std::make_tuple(RequestMsgCodes::unknown, 0));

    std::array<uint8_t, 3> txbuff;

    CHECK(Protocol::EncodeRequest(RequestMsg(code, value), txbuff.data()) == 3);
    CHECK(txbuff[0] == (uint8_t)code);
    CHECK(txbuff[1] == value + '0');
    CHECK(txbuff[2] == '\n');
}

TEST_CASE("protocol::EncodeResponseCmdAR", "[protocol]") {
    using namespace modules;

    auto requestMsg = GENERATE(
        RequestMsg(RequestMsgCodes::Button, 0),
        RequestMsg(RequestMsgCodes::Button, 1),
        RequestMsg(RequestMsgCodes::Button, 2),

        RequestMsg(RequestMsgCodes::Cut, 0),

        RequestMsg(RequestMsgCodes::Eject, 0),
        RequestMsg(RequestMsgCodes::Eject, 1),
        RequestMsg(RequestMsgCodes::Eject, 2),
        RequestMsg(RequestMsgCodes::Eject, 3),
        RequestMsg(RequestMsgCodes::Eject, 4),

        RequestMsg(RequestMsgCodes::Load, 0),
        RequestMsg(RequestMsgCodes::Load, 1),
        RequestMsg(RequestMsgCodes::Load, 2),
        RequestMsg(RequestMsgCodes::Load, 3),
        RequestMsg(RequestMsgCodes::Load, 4),

        RequestMsg(RequestMsgCodes::Mode, 0),
        RequestMsg(RequestMsgCodes::Mode, 1),

        RequestMsg(RequestMsgCodes::Tool, 0),
        RequestMsg(RequestMsgCodes::Tool, 1),
        RequestMsg(RequestMsgCodes::Tool, 2),
        RequestMsg(RequestMsgCodes::Tool, 3),
        RequestMsg(RequestMsgCodes::Tool, 4),

        RequestMsg(RequestMsgCodes::Unload, 0),

        RequestMsg(RequestMsgCodes::Wait, 0));

    auto responseStatus = GENERATE(ResponseMsgParamCodes::Accepted, ResponseMsgParamCodes::Rejected);

    std::array<uint8_t, 8> txbuff;
    uint8_t msglen = Protocol::EncodeResponseCmdAR(requestMsg, responseStatus, txbuff.data());

    CHECK(msglen == 5);
    CHECK(txbuff[0] == (uint8_t)requestMsg.code);
    CHECK(txbuff[1] == requestMsg.value + '0');
    CHECK(txbuff[2] == ' ');
    CHECK(txbuff[3] == (uint8_t)responseStatus);
    CHECK(txbuff[4] == '\n');
}

TEST_CASE("protocol::EncodeResponseReadFINDA", "[protocol]") {
    using namespace modules;
    auto requestMsg = RequestMsg(RequestMsgCodes::Finda, 0);

    uint8_t findaStatus = GENERATE(0, 1);

    std::array<uint8_t, 8> txbuff;
    uint8_t msglen = Protocol::EncodeResponseReadFINDA(requestMsg, findaStatus, txbuff.data());

    CHECK(msglen == 6);
    CHECK(txbuff[0] == (uint8_t)requestMsg.code);
    CHECK(txbuff[1] == requestMsg.value + '0');
    CHECK(txbuff[2] == ' ');
    CHECK(txbuff[3] == (uint8_t)ResponseMsgParamCodes::Accepted);
    CHECK(txbuff[4] == findaStatus + '0');
    CHECK(txbuff[5] == '\n');
}

TEST_CASE("protocol::EncodeResponseVersion", "[protocol]") {
    using namespace modules;

    std::uint8_t versionQueryType = GENERATE(0, 1, 2, 3);
    auto requestMsg = RequestMsg(RequestMsgCodes::Version, versionQueryType);

    auto version = GENERATE(0, 1, 2, 3, 4, 10, 11, 12, 20, 99, 100, 101, 255);

    std::array<uint8_t, 8> txbuff;
    uint8_t msglen = Protocol::EncodeResponseVersion(requestMsg, version, txbuff.data());

    CHECK(msglen <= 8);
    CHECK(txbuff[0] == (uint8_t)requestMsg.code);
    CHECK(txbuff[1] == requestMsg.value + '0');
    CHECK(txbuff[2] == ' ');
    CHECK(txbuff[3] == (uint8_t)ResponseMsgParamCodes::Accepted);

    if (version < 10) {
        CHECK(txbuff[4] == version + '0');
    } else if (version < 100) {
        CHECK(txbuff[4] == version / 10 + '0');
        CHECK(txbuff[5] == version % 10 + '0');
    } else {
        CHECK(txbuff[4] == version / 100 + '0');
        CHECK(txbuff[5] == (version / 10) % 10 + '0');
        CHECK(txbuff[6] == version % 10 + '0');
    }

    CHECK(txbuff[msglen - 1] == '\n');
}

TEST_CASE("protocol::EncodeResponseQueryOperation", "[protocol]") {
    using namespace modules;

    auto requestMsg = GENERATE(
        RequestMsg(RequestMsgCodes::Cut, 0),

        RequestMsg(RequestMsgCodes::Eject, 0),
        RequestMsg(RequestMsgCodes::Eject, 1),
        RequestMsg(RequestMsgCodes::Eject, 2),
        RequestMsg(RequestMsgCodes::Eject, 3),
        RequestMsg(RequestMsgCodes::Eject, 4),

        RequestMsg(RequestMsgCodes::Load, 0),
        RequestMsg(RequestMsgCodes::Load, 1),
        RequestMsg(RequestMsgCodes::Load, 2),
        RequestMsg(RequestMsgCodes::Load, 3),
        RequestMsg(RequestMsgCodes::Load, 4),

        RequestMsg(RequestMsgCodes::Tool, 0),
        RequestMsg(RequestMsgCodes::Tool, 1),
        RequestMsg(RequestMsgCodes::Tool, 2),
        RequestMsg(RequestMsgCodes::Tool, 3),
        RequestMsg(RequestMsgCodes::Tool, 4),

        RequestMsg(RequestMsgCodes::Unload, 0),

        RequestMsg(RequestMsgCodes::Wait, 0));

    auto responseStatus = GENERATE(ResponseMsgParamCodes::Processing, ResponseMsgParamCodes::Error, ResponseMsgParamCodes::Finished);

    auto value = GENERATE(0, 1, 2, 3, 10, 11, 99, 100, 101, 102, 200, 255);

    std::array<uint8_t, 8> txbuff;
    uint8_t msglen = Protocol::EncodeResponseQueryOperation(requestMsg, responseStatus, value, txbuff.data());

    CHECK(msglen <= 8);
    CHECK(txbuff[0] == (uint8_t)requestMsg.code);
    CHECK(txbuff[1] == requestMsg.value + '0');
    CHECK(txbuff[2] == ' ');
    CHECK(txbuff[3] == (uint8_t)responseStatus);

    if (responseStatus == ResponseMsgParamCodes::Finished) {
        CHECK(txbuff[4] == '\n');
        CHECK(msglen == 5);
    } else {
        if (value < 10) {
            CHECK(txbuff[4] == value + '0');
        } else if (value < 100) {
            CHECK(txbuff[4] == value / 10 + '0');
            CHECK(txbuff[5] == value % 10 + '0');
        } else {
            CHECK(txbuff[4] == value / 100 + '0');
            CHECK(txbuff[5] == (value / 10) % 10 + '0');
            CHECK(txbuff[6] == value % 10 + '0');
        }

        CHECK(txbuff[msglen - 1] == '\n');
    }
}

TEST_CASE("protocol::DecodeRequest", "[protocol]") {
    using namespace modules;
    Protocol p;
    const char *rxbuff = GENERATE(
        "B0\n", "B1\n", "B2\n",
        "E0\n", "E1\n", "E2\n", "E3\n", "E4\n",
        "K0\n",
        "L0\n", "L1\n", "L2\n", "L3\n", "L4\n",
        "M0\n", "M1\n",
        "P0\n",
        "Q0\n",
        "S0\n", "S1\n", "S2\n", "S3\n",
        "T0\n", "T1\n", "T2\n", "T3\n",
        "U0\n",
        "W0\n",
        "X0\n");

    const char *pc = rxbuff;
    for (;;) {
        uint8_t c = *pc++;
        if (c == 0) {
            // end of input test data
            break;
        } else if (c == '\n') {
            // regular end of message line
            CHECK(p.DecodeRequest(c) == Protocol::DecodeStatus::MessageCompleted);
        } else {
            CHECK(p.DecodeRequest(c) == Protocol::DecodeStatus::NeedMoreData);
        }
    }

    // check the message type
    const RequestMsg &rq = p.GetRequestMsg();
    CHECK((uint8_t)rq.code == rxbuff[0]);
    CHECK(rq.value == rxbuff[1] - '0');
}

TEST_CASE("protocol::DecodeResponseReadFinda", "[protocol]") {
    using namespace modules;
    Protocol p;
    const char *rxbuff = GENERATE(
        "P0 A0\n",
        "P0 A1\n");

    const char *pc = rxbuff;
    for (;;) {
        uint8_t c = *pc++;
        if (c == 0) {
            // end of input test data
            break;
        } else if (c == '\n') {
            // regular end of message line
            CHECK(p.DecodeResponse(c) == Protocol::DecodeStatus::MessageCompleted);
        } else {
            CHECK(p.DecodeResponse(c) == Protocol::DecodeStatus::NeedMoreData);
        }
    }

    // check the message type
    const ResponseMsg &rsp = p.GetResponseMsg();
    CHECK((uint8_t)rsp.request.code == rxbuff[0]);
    CHECK(rsp.request.value == rxbuff[1] - '0');
    CHECK((uint8_t)rsp.paramCode == rxbuff[3]);
    CHECK((uint8_t)rsp.paramValue == rxbuff[4] - '0');
}

TEST_CASE("protocol::DecodeResponseQueryOperation", "[protocol]") {
    using namespace modules;
    Protocol p;
    const char *cmdReference = GENERATE(
        "E0", "E1", "E2", "E3", "E4",
        "K0",
        "L0", "L1", "L2", "L3", "L4",
        "T0", "T1", "T2", "T3",
        "U0",
        "W0");

    const char *status = GENERATE(
        "P0", "P1", "E0", "E1", "E9", "F");

    std::string rxbuff(cmdReference);
    rxbuff += ' ';
    rxbuff += status;
    rxbuff += '\n';

    const char *pc = rxbuff.c_str();
    for (;;) {
        uint8_t c = *pc++;
        if (c == 0) {
            // end of input test data
            break;
        } else if (c == '\n') {
            // regular end of message line
            CHECK(p.DecodeResponse(c) == Protocol::DecodeStatus::MessageCompleted);
        } else {
            CHECK(p.DecodeResponse(c) == Protocol::DecodeStatus::NeedMoreData);
        }
    }

    // check the message type
    const ResponseMsg &rsp = p.GetResponseMsg();
    CHECK((uint8_t)rsp.request.code == rxbuff[0]);
    CHECK(rsp.request.value == rxbuff[1] - '0');
    CHECK((uint8_t)rsp.paramCode == rxbuff[3]);
    if ((uint8_t)rsp.paramCode != (uint8_t)ResponseMsgParamCodes::Finished) {
        CHECK((uint8_t)rsp.paramValue == rxbuff[4] - '0');
    }
}

TEST_CASE("protocol::DecodeRequestErrors", "[protocol]") {
    using namespace modules;
    Protocol p;
    const char b0[] = "b0";
    CHECK(p.DecodeRequest(b0[0]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(b0[1]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);

    // reset protokol decoder
    CHECK(p.DecodeRequest('\n') == Protocol::DecodeStatus::MessageCompleted);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);

    const char B1_[] = "B1 \n";
    CHECK(p.DecodeRequest(B1_[0]) == Protocol::DecodeStatus::NeedMoreData);
    CHECK(p.DecodeRequest(B1_[1]) == Protocol::DecodeStatus::NeedMoreData);
    CHECK(p.DecodeRequest(B1_[2]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(B1_[3]) == Protocol::DecodeStatus::MessageCompleted);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);

    const char _B2[] = " B2\n";
    CHECK(p.DecodeRequest(_B2[0]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(_B2[1]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(_B2[2]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(_B2[3]) == Protocol::DecodeStatus::MessageCompleted);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);

    const char _B0_[] = " B0 ";
    CHECK(p.DecodeRequest(_B0_[0]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(_B0_[1]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(_B0_[2]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(_B0_[3]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest('\n') == Protocol::DecodeStatus::MessageCompleted);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
}

TEST_CASE("protocol::DecodeResponseErrors", "[protocol]") {
    using namespace modules;
    Protocol p;

    const char b0[] = "b0 A\n";
    CHECK(p.DecodeRequest(b0[0]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(b0[1]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(b0[2]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(b0[3]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(b0[4]) == Protocol::DecodeStatus::MessageCompleted);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);

    const char b1[] = "b0A\n";
    CHECK(p.DecodeRequest(b1[0]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(b1[1]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(b1[2]) == Protocol::DecodeStatus::Error);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
    CHECK(p.DecodeRequest(b1[3]) == Protocol::DecodeStatus::MessageCompleted);
    CHECK(p.GetRequestMsg().code == RequestMsgCodes::unknown);
}

// Beware - this test makes 18M+ combinations, run only when changing the implementation of the codec
// Therefore it is disabled [.] by default
TEST_CASE("protocol::DecodeResponseErrorsCross", "[protocol][.]") {
    using namespace modules;
    Protocol p;

    const char *validInitialSpaces = "";
    const char *invalidInitialSpaces = GENERATE(" ", "  ");
    bool viInitialSpace = GENERATE(true, false);

    const char *validReqCode = GENERATE("B", "E", "K", "L", "M", "P", "Q", "S", "T", "U", "W", "X");
    const char *invalidReqCode = GENERATE("A", "R", "F");
    bool viReqCode = GENERATE(true, false);

    const char *validReqValue = GENERATE("0", "1", "2", "3", "4");
    // these are silently accepted
    //    const char *invalidReqValue = GENERATE(/*"5", */"10", "100");
    //    bool viReqValue = GENERATE(true, false);

    const char *validSpace = " ";
    const char *invalidSpace = GENERATE("", "  ");
    bool viSpace = GENERATE(true, false);

    const char *validRspCode = GENERATE("A", "R", "P", "E", "F");
    const char *invalidRspCode = GENERATE("B", "K", "L", "M", "Q");
    bool viRspCode = GENERATE(true, false);

    const char *validRspValue = GENERATE("0", "1", "2", "3", "10", "11", "100", "255");

    const char *validTerminatingSpaces = "";
    const char *invalidTerminatingSpaces = GENERATE(" ", "  ");
    bool viTerminatingSpaces = GENERATE(true, false);

    // skip valid combinations
    std::string msg;
    msg += viInitialSpace ? validInitialSpaces : invalidInitialSpaces;
    msg += viReqCode ? validReqCode : invalidReqCode;
    msg += validReqValue; //viReqValue ? validReqValue : invalidReqValue;
    msg += viSpace ? validSpace : invalidSpace;
    const char *rspCode = viRspCode ? validRspCode : invalidRspCode;
    msg += rspCode;
    if (rspCode[0] == 'F') {
        // this one doesn't have any value behind
    } else {
        msg += validRspValue;
    }
    msg += viTerminatingSpaces ? validTerminatingSpaces : invalidTerminatingSpaces;
    msg += '\n';

    bool shouldPass = viInitialSpace && viReqCode && /*viReqValue && */ viSpace && viRspCode && viTerminatingSpaces;
    bool failed = false;
    std::for_each(msg.cbegin(), msg.cend(), [&](uint8_t c) {
        if (p.DecodeResponse(c) == Protocol::DecodeStatus::Error) {
            failed = true;
        }
    });
    CHECK(failed != shouldPass); // it must have failed!
    if (failed) {
        CHECK(p.GetResponseMsg().paramCode == ResponseMsgParamCodes::unknown);
    }
}
