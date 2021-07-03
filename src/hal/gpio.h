#pragma once
#include <inttypes.h>

#ifdef __AVR__
#include <avr/io.h>
#endif

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
    // No constructor here in order to allow brace-initialization in old
    // gcc versions/standards
    GPIO_TypeDef *const port;
    const uint8_t pin;
};

__attribute__((always_inline)) inline void WritePin(const GPIO_pin portPin, Level level) {
    if (level == Level::high)
        portPin.port->PORTx |= (1 << portPin.pin);
    else
        portPin.port->PORTx &= ~(1 << portPin.pin);
}

__attribute__((always_inline)) inline Level ReadPin(const GPIO_pin portPin) {
    return (Level)(portPin.port->PINx & (1 << portPin.pin));
}

__attribute__((always_inline)) inline void TogglePin(const GPIO_pin portPin) {
    portPin.port->PINx |= (1 << portPin.pin);
}

__attribute__((always_inline)) inline void Init(const GPIO_pin portPin, GPIO_InitTypeDef GPIO_Init) {
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

#ifdef __AVR__
#define GPIOA ((hal::gpio::GPIO_TypeDef *)&PINA)
#define GPIOB ((hal::gpio::GPIO_TypeDef *)&PINB)
#define GPIOC ((hal::gpio::GPIO_TypeDef *)&PINC)
#define GPIOD ((hal::gpio::GPIO_TypeDef *)&PIND)
#define GPIOE ((hal::gpio::GPIO_TypeDef *)&PINE)
#define GPIOF ((hal::gpio::GPIO_TypeDef *)&PINF)
#define GPIOG ((hal::gpio::GPIO_TypeDef *)&PING)
#define GPIOH ((hal::gpio::GPIO_TypeDef *)&PINH)
#define GPIOJ ((hal::gpio::GPIO_TypeDef *)&PINJ)
#define GPIOK ((hal::gpio::GPIO_TypeDef *)&PINK)
#define GPIOL ((hal::gpio::GPIO_TypeDef *)&PINL)
#else

// stub entries
extern hal::gpio::GPIO_TypeDef _GPIOA;
extern hal::gpio::GPIO_TypeDef _GPIOB;
extern hal::gpio::GPIO_TypeDef _GPIOC;
extern hal::gpio::GPIO_TypeDef _GPIOD;
extern hal::gpio::GPIO_TypeDef _GPIOE;
extern hal::gpio::GPIO_TypeDef _GPIOF;
extern hal::gpio::GPIO_TypeDef _GPIOG;
extern hal::gpio::GPIO_TypeDef _GPIOH;
extern hal::gpio::GPIO_TypeDef _GPIOJ;
extern hal::gpio::GPIO_TypeDef _GPIOK;
extern hal::gpio::GPIO_TypeDef _GPIOL;

#define GPIOA (&_GPIOB)
#define GPIOB (&_GPIOB)
#define GPIOC (&_GPIOB)
#define GPIOD (&_GPIOB)
#define GPIOE (&_GPIOB)
#define GPIOF (&_GPIOB)
#define GPIOG (&_GPIOB)
#define GPIOH (&_GPIOB)
#define GPIOJ (&_GPIOB)
#define GPIOK (&_GPIOB)
#define GPIOL (&_GPIOB)

#endif
