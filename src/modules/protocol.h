#pragma once

/// MMU protocol implementation
/// See description of the new protocol in the MMU 2021 doc
/// @@TODO possibly add some checksum to verify the correctness of messages

namespace modules {

/// @@TODO define/improve this simple message structure that fits our domain
struct Msg {
    uint8_t code;
    uint8_t value;
};

/// Protocol class is responsible for creating/decoding messages in Rx/Tx buffer
class Protocol {

public:
    /// Decodes message in rxbuff
    /// @returns decoded message structure
    Msg Decode(const uint8_t *rxbuff);
    /// Encodes message msg into txbuff memory
    /// It is expected the txbuff is large enough to fit the message
    void Encode(uint8_t *txbuff, const Msg &msg);
};

} // namespace modules
