#pragma once
#include <inttypes.h>
#include <avr/io.h>

namespace hal {
namespace gpio {

    struct GPIO_TypeDef {
        volatile uint8_t PINx;
        volatile uint8_t DDRx;
        volatile uint8_t PORTx;
    };

    enum class Mode : uint8_t {
        input = 0,
        output,
    };

    enum class Pull : uint8_t {
        none = 0,
        up,
        down, //not available on the AVR
    };

    enum class Level : uint8_t {
        low = 0,
        high,
    };

    struct GPIO_InitTypeDef {
        Mode mode;
        Pull pull;
        Level level;
        inline GPIO_InitTypeDef(Mode mode, Pull pull)
            : mode(mode)
            , pull(pull) {};
        inline GPIO_InitTypeDef(Mode mode, Level level)
            : mode(mode)
            , level(level) {};
    };

    struct GPIO_pin {
        GPIO_TypeDef *const port;
        const uint8_t pin;
        inline GPIO_pin(GPIO_TypeDef *const port, const uint8_t pin)
            : port(port)
            , pin(pin) {};
    };

    inline void WritePin(const GPIO_pin portPin, Level level) {
        if (level == Level::high)
            portPin.port->PORTx |= (1 << portPin.pin);
        else
            portPin.port->PORTx &= ~(1 << portPin.pin);
    }

    inline Level ReadPin(const GPIO_pin portPin) {
        return (Level)(portPin.port->PINx & (1 << portPin.pin));
    }

    inline void TogglePin(const GPIO_pin portPin) {
        portPin.port->PINx |= (1 << portPin.pin);
    }

    inline void Init(const GPIO_pin portPin, GPIO_InitTypeDef GPIO_Init) {
        if (GPIO_Init.mode == Mode::output) {
            WritePin(portPin, GPIO_Init.level);
            portPin.port->DDRx |= (1 << portPin.pin);
        } else {
            portPin.port->DDRx &= ~(1 << portPin.pin);
            WritePin(portPin, (Level)GPIO_Init.pull);
        }
    }

}
}

#define GPIOB ((hal::gpio::GPIO_TypeDef *)&PINB)
#define GPIOC ((hal::gpio::GPIO_TypeDef *)&PINC)
#define GPIOD ((hal::gpio::GPIO_TypeDef *)&PIND)
#define GPIOE ((hal::gpio::GPIO_TypeDef *)&PINE)
#define GPIOF ((hal::gpio::GPIO_TypeDef *)&PINF)
