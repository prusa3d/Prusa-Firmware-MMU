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
