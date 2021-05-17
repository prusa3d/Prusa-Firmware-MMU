#include "../spi.h"

namespace hal {
namespace spi {

    // void Init(SPI_TypeDef *const hspi, SPI_InitTypeDef *const conf)
    // {
    //     using namespace hal;
    //     // gpio::Init(conf->miso_pin, gpio::GPIO_InitTypeDef(gpio::Mode::input, gpio::Pull::none));
    //     // gpio::Init(conf->mosi_pin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::low));
    //     // gpio::Init(conf->sck_pin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::low));
    //     // gpio::Init(conf->ss_pin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::high));

    //     const uint8_t spi2x = (conf->prescaler == 7) ? 0 : (conf->prescaler & 0x01);
    //     const uint8_t spr = ((conf->prescaler - 1) >> 1) & 0x03;

    //     hspi->SPCRx = (0 << SPIE) | (1 << SPE) | (0 << DORD) | (1 << MSTR) | (0 << CPOL) | (0 << CPHA) | (spr << SPR0);
    //     hspi->SPSRx = (spi2x << SPI2X);
    // }
}
}
