#pragma once
#include "hal/gpio.h"

/// pin definitions
#define GPIO_PIN(port, pin) hal::gpio::GPIO_pin{port, pin}

#define TMC2130_SPI_MISO_PIN GPIO_PIN(GPIOB, 3)
#define TMC2130_SPI_MOSI_PIN GPIO_PIN(GPIOB, 2)
#define TMC2130_SPI_SCK_PIN  GPIO_PIN(GPIOB, 1)
#define TMC2130_SPI_SS_PIN   GPIO_PIN(GPIOB, 0)

#define SHR16_DATA  GPIO_PIN(GPIOB, 5) ///DS
#define SHR16_LATCH GPIO_PIN(GPIOB, 6) ///STCP
#define SHR16_CLOCK GPIO_PIN(GPIOC, 7) ///SHCP

#define USART_RX GPIO_PIN(GPIOD, 2)
#define USART_TX GPIO_PIN(GPIOD, 3)
