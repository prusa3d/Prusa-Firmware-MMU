#pragma once

/// UART interface
/// @@TODO decide, if this class will behave like a singleton, or there will be multiple classes
/// for >1 UART interfaces

namespace hal {
class UART {

public:
    /// @returns current character from the UART without extracting it from the read buffer
    uint8_t Peek() const;
    /// @returns true if there are no bytes to be read
    bool ReadEmpty() const;
    /// @returns current character from the UART and extracts it from the read buffer
    uint8_t Read();

    /// @param c character to be pushed into the TX buffer (to be sent)
    void Write(uint8_t c);
    /// @returns true if there is at least one byte free in the TX buffer (i.e. some space to add a character to be sent)
    bool CanWrite() const;
    /// blocks until the TX buffer was successfully transmitted
    void Flush();

private:
    /// implementation of the receive ISR's body
    void ISR_RX();
    /// implementation of the transmit ISR's body
    void ISR_TX();
};

} // namespace hal
