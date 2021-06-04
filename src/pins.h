#pragma once
#include "hal/gpio.h"

/// pin definitions

#define TMC2130_SPI_MISO_PIN GPIOB, 3
#define TMC2130_SPI_MOSI_PIN GPIOB, 2
#define TMC2130_SPI_SCK_PIN GPIOB, 1
#define TMC2130_SPI_SS_PIN GPIOB, 0

#define SHR16_DATA GPIOB, 5 ///DS
#define SHR16_LATCH GPIOB, 6 ///STCP
#define SHR16_CLOCK GPIOB, 7 ///SHCP
