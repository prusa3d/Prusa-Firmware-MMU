#pragma once
#include <inttypes.h>
#include "gpio.h"

namespace hal {

/// SPI interface
namespace spi {

struct SPI_TypeDef {
    volatile uint8_t SPCRx;
    volatile uint8_t SPSRx;
    volatile uint8_t SPDRx;
};

struct SPI_InitTypeDef {
    hal::gpio::GPIO_pin miso_pin;
    hal::gpio::GPIO_pin mosi_pin;
    hal::gpio::GPIO_pin sck_pin;
    hal::gpio::GPIO_pin ss_pin;
    uint8_t prescaler;
    uint8_t cpha;
    uint8_t cpol;
};

__attribute__((always_inline)) inline void Init(SPI_TypeDef *const hspi, SPI_InitTypeDef *const conf) {
    using namespace hal;
    gpio::Init(conf->miso_pin, gpio::GPIO_InitTypeDef(gpio::Mode::input, gpio::Pull::none));
    gpio::Init(conf->mosi_pin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::low));
    gpio::Init(conf->sck_pin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::low));
    gpio::Init(conf->ss_pin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::high)); //the AVR requires this pin to be an output for SPI master mode to work properly.

    const uint8_t spi2x = (conf->prescaler == 7) ? 0 : (conf->prescaler & 0x01);
    const uint8_t spr = ((conf->prescaler - 1) >> 1) & 0x03;

    hspi->SPCRx = (0 << SPIE) | (1 << SPE) | (0 << DORD) | (1 << MSTR) | ((conf->cpol & 0x01) << CPOL) | ((conf->cpha & 0x01) << CPHA) | (spr << SPR0);
    hspi->SPSRx = (spi2x << SPI2X);
}

__attribute__((always_inline)) inline uint8_t TxRx(SPI_TypeDef *const hspi, uint8_t val) {
    hspi->SPDRx = val;
    while (!(hspi->SPSRx & (1 << SPIF)))
        ;
    return hspi->SPDRx;
}
}
}

#define SPI0 ((hal::spi::SPI_TypeDef *)&SPCR)
