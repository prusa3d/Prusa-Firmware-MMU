#include "../tmc2130.h"

namespace hal {
namespace tmc2130 {

TMC2130::TMC2130(const MotorParams &params,
    const MotorCurrents &currents,
    MotorMode mode) {
    // TODO
}

void TMC2130::Init(const MotorParams &params) {
    // TODO
}

uint32_t TMC2130::ReadRegister(const MotorParams &params, Registers reg) {
    uint8_t pData[5] = {(uint8_t)reg};
	_spi_tx_rx(params, pData);
	// _handle_spi_status(pData[0]); /// could be outdated. Safer not to use it.
	pData[0] = 0;
	_spi_tx_rx(params, pData);
	_handle_spi_status(pData[0]);
	return ((uint32_t)pData[1] << 24 | (uint32_t)pData[2] << 16 | (uint32_t)pData[3] << 8 | (uint32_t)pData[4]);
}

void TMC2130::WriteRegister(const MotorParams &params, Registers reg, uint32_t data) {
	uint8_t pData[5] = {(uint8_t)((uint8_t)(reg) | 0x80), (uint8_t)(data >> 24), (uint8_t)(data >> 16), (uint8_t)(data >> 8), (uint8_t)data};
	_spi_tx_rx(params, pData);
	_handle_spi_status(pData[0]);
}

void TMC2130::_spi_tx_rx(const MotorParams &params, uint8_t (&pData)[5]) {
    hal::gpio::WritePin(params.csPin, hal::gpio::Level::low);
    for (uint8_t i = 0; i < sizeof(pData); i++)
        pData[i] = hal::spi::TxRx(params.spi, pData[i]);
    hal::gpio::WritePin(params.csPin, hal::gpio::Level::high);
}

void TMC2130::_handle_spi_status(uint8_t status) {
    spi_status |= status & 0x03; /// update reset_flag and driver_error
}

} // namespace tmc2130
} // namespace hal
