/// @file pins.h
#pragma once
#include "hal/gpio.h"

/// pin definitions
static constexpr hal::gpio::GPIO_pin TMC2130_SPI_MISO_PIN = { GPIOB, (1 << 3) };
static constexpr hal::gpio::GPIO_pin TMC2130_SPI_MOSI_PIN = { GPIOB, (1 << 2) };
static constexpr hal::gpio::GPIO_pin TMC2130_SPI_SCK_PIN = { GPIOB, (1 << 1) };
static constexpr hal::gpio::GPIO_pin TMC2130_SPI_SS_PIN = { GPIOB, (1 << 0) };

static constexpr hal::gpio::GPIO_pin SHR16_DATA = { GPIOB, (1 << 5) }; ///DS
static constexpr hal::gpio::GPIO_pin SHR16_LATCH = { GPIOB, (1 << 6) }; ///STCP
static constexpr hal::gpio::GPIO_pin SHR16_CLOCK = { GPIOC, (1 << 7) }; ///SHCP

static constexpr hal::gpio::GPIO_pin USART_RX = { GPIOD, (1 << 2) };
static constexpr hal::gpio::GPIO_pin USART_TX = { GPIOD, (1 << 3) };

static constexpr hal::gpio::GPIO_pin PULLEY_CS_PIN = { GPIOC, (1 << 6) };
static constexpr hal::gpio::GPIO_pin PULLEY_SG_PIN = { GPIOF, (1 << 4) };
static constexpr hal::gpio::GPIO_pin PULLEY_STEP_PIN = { GPIOB, (1 << 4) };

static constexpr hal::gpio::GPIO_pin SELECTOR_CS_PIN = { GPIOD, (1 << 7) };
static constexpr hal::gpio::GPIO_pin SELECTOR_SG_PIN = { GPIOF, (1 << 1) };
static constexpr hal::gpio::GPIO_pin SELECTOR_STEP_PIN = { GPIOD, (1 << 4) };

static constexpr hal::gpio::GPIO_pin IDLER_CS_PIN = { GPIOB, (1 << 7) };
static constexpr hal::gpio::GPIO_pin IDLER_SG_PIN = { GPIOF, (1 << 0) };
static constexpr hal::gpio::GPIO_pin IDLER_STEP_PIN = { GPIOD, (1 << 6) };

static constexpr hal::gpio::GPIO_pin FINDA_PIN = { GPIOF, (1 << 6) }; /// PF6     A1      ADC6/TDI
