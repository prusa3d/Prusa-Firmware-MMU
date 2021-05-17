#pragma once
#include <inttypes.h>
#include "gpio.h"

/// SPI interface

namespace hal {
namespace spi {
    struct SPI_TypeDef {
        volatile uint8_t SPCRx;
        volatile uint8_t SPSRx;
        volatile uint8_t SPDRx;
    };

    struct SPI_InitTypeDef {
        // hal::gpio::GPIO_pin miso_pin;
        // hal::gpio::GPIO_pin mosi_pin;
        // hal::gpio::GPIO_pin sck_pin;
        // hal::gpio::GPIO_pin ss_pin;
        uint8_t prescaler;
    };

    // void Init(SPI_TypeDef *const hspi, SPI_InitTypeDef *const conf);

    inline void Init(SPI_TypeDef *const hspi, uint8_t prescaler) {
        using namespace hal;
        // gpio::Init(conf->miso_pin, gpio::GPIO_InitTypeDef(gpio::Mode::input, gpio::Pull::none));
        // gpio::Init(conf->mosi_pin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::low));
        // gpio::Init(conf->sck_pin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::low));
        // gpio::Init(conf->ss_pin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::high));

        const uint8_t spi2x = (prescaler == 7) ? 0 : (prescaler & 0x01);
        const uint8_t spr = ((prescaler - 1) >> 1) & 0x03;

        hspi->SPCRx = (0 << SPIE) | (1 << SPE) | (0 << DORD) | (1 << MSTR) | (0 << CPOL) | (0 << CPHA) | (spr << SPR0);
        hspi->SPSRx = (spi2x << SPI2X);
    }

}
}

#define SPI0 ((hal::spi::SPI_TypeDef *)&SPCR)
