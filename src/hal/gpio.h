#include <inttypes.h>
#include <avr/io.h>

namespace hal {
namespace gpio {

    typedef struct
    {
        volatile uint8_t PINx;
        volatile uint8_t DDRx;
        volatile uint8_t PORTx;
    } GPIO_TypeDef;

    enum class Mode {
        input = 0,
        output,
    };

    enum class Pull {
        none = 0,
        up,
        down, //not available on the AVR
    };

    enum class Level {
        low = 0,
        high,
    };

    struct GPIO_InitTypeDef {
        Mode mode;
        Pull pull;
        Level level;
        inline GPIO_InitTypeDef()
            : mode(Mode::input)
            , pull(Pull::none) {};
        inline GPIO_InitTypeDef(Mode mode, Pull pull)
            : mode(mode)
            , pull(pull) {};
        inline GPIO_InitTypeDef(Mode mode, Level level)
            : mode(mode)
            , level(level) {};
    };

    inline void WritePin(GPIO_TypeDef *const port, const uint8_t pin, Level level) {
        if (level == Level::high)
            port->PORTx |= (1 << pin);
        else
            port->PORTx &= ~(1 << pin);
    }

    inline Level ReadPin(GPIO_TypeDef *const port, const uint8_t pin) {
        return (Level)(port->PINx & (1 << pin));
    }

    inline void TogglePin(GPIO_TypeDef *const port, const uint8_t pin) {
        port->PINx |= (1 << pin);
    }

    inline void Init(GPIO_TypeDef *const port, const uint8_t pin, GPIO_InitTypeDef GPIO_Init) {
        if (GPIO_Init.mode == Mode::output) {
            WritePin(port, pin, GPIO_Init.level);
            port->DDRx |= (1 << pin);
        } else {
            port->DDRx &= ~(1 << pin);
            WritePin(port, pin, (Level)GPIO_Init.pull);
        }
    }

}
}

#define GPIOB ((hal::gpio::GPIO_TypeDef *)&PINB)
#define GPIOC ((hal::gpio::GPIO_TypeDef *)&PINC)
#define GPIOD ((hal::gpio::GPIO_TypeDef *)&PIND)
#define GPIOE ((hal::gpio::GPIO_TypeDef *)&PINE)
#define GPIOF ((hal::gpio::GPIO_TypeDef *)&PINF)
