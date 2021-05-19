#pragma once
#include <inttypes.h>
#include <avr/io.h>
#include "gpio.h"
#include "circle_buffer.hpp"
/// USART interface
/// @@TODO decide, if this class will behave like a singleton, or there will be multiple classes
/// for >1 USART interfaces

namespace hal {
class USART {
public:
    struct USART_TypeDef {
        volatile uint8_t UCSRxA;
        volatile uint8_t UCSRxB;
        volatile uint8_t UCSRxC;
        volatile uint8_t UCSRxD;
        volatile uint16_t UBRRx;
        volatile uint8_t UDRx;
    };

    struct USART_InitTypeDef {
        hal::gpio::GPIO_pin rx_pin;
        hal::gpio::GPIO_pin tx_pin;
        uint32_t baudrate;
    };

    /// @returns current character from the UART without extracting it from the read buffer
    uint8_t Peek() const {
        return rx_buf.GetFirstIfAble();
    }
    /// @returns true if there are no bytes to be read
    bool ReadEmpty() const {
        return rx_buf.IsEmpty();
    }
    /// @returns current character from the UART and extracts it from the read buffer
    uint8_t Read();

    /// @param c character to be pushed into the TX buffer (to be sent)
    void Write(uint8_t c);
    /// @param str c string to be sent. NL is appended
    void puts(const char *str);
    /// @returns true if there is at least one byte free in the TX buffer (i.e. some space to add a character to be sent)
    bool CanWrite() const {
        return tx_buf.CanPush();
    }
    /// blocks until the TX buffer was successfully transmitted
    void Flush();

    /// Initializes USART interface
    __attribute__((always_inline)) inline void Init(USART_InitTypeDef *const conf) {
        gpio::Init(conf->rx_pin, gpio::GPIO_InitTypeDef(gpio::Mode::input, gpio::Level::low));
        gpio::Init(conf->tx_pin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::low));
        husart->UBRRx = (((double)(F_CPU)) / (((double)(conf->baudrate)) * 8.0) - 1.0 + 0.5);
        husart->UCSRxA = (1 << 1); // Set double baudrate setting. Clear all other status bits/flags
        // husart->UCSRxC |= (1 << 3); // 2 stop bits. Preserve data size setting
        husart->UCSRxD = 0; //disable hardware flow control. This register is reserved on all AVR devides with USART.
        husart->UCSRxB = (1 << 3) | (1 << 4) | (1 << 7); // Turn on the transmission and reception circuitry and enable the RX interrupt
    }

    /// implementation of the receive ISR's body
    __attribute__((always_inline)) inline void ISR_RX() {
        if (husart->UCSRxA & (1 << 4)) {
            (void)husart->UDRx;
        } else {
            rx_buf.push_back_DontRewrite(husart->UDRx);
        }
    }
    /// implementation of the transmit ISR's body
    __attribute__((always_inline)) inline void ISR_UDRE() {
        uint8_t c = 0;
        tx_buf.ConsumeFirst(c);
        husart->UDRx = c;

        // clear the TXC bit -- "can be cleared by writing a one to its bit
        // location". This makes sure flush() won't return until the bytes
        // actually got written
        husart->UCSRxA |= (1 << 6);

        if (tx_buf.IsEmpty())
            husart->UCSRxB &= ~(1 << 5); // disable UDRE interrupt
    }

    USART(USART_TypeDef *husart)
        : husart(husart) {};

private:
    // IO base address
    USART_TypeDef *husart;
    bool _written;

    CircleBuffer<uint8_t, 32> tx_buf;
    CircleBuffer<uint8_t, 32> rx_buf;
};

} // namespace hal

#define USART1 ((hal::USART::USART_TypeDef *)&UCSR1A)
extern hal::USART usart1;
