#include "../tmc2130.h"

namespace hal {
namespace tmc2130 {

TMC2130::TMC2130(const MotorParams &params,
    const MotorCurrents &currents,
    MotorMode mode)
    : currents(currents) {
    // TODO
}

bool TMC2130::Init(const MotorParams &params) {
    gpio::Init(params.csPin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::high));
    gpio::Init(params.sgPin, gpio::GPIO_InitTypeDef(gpio::Mode::input, gpio::Pull::up));
    gpio::Init(params.stepPin, gpio::GPIO_InitTypeDef(gpio::Mode::output, gpio::Level::low));

    ///check for compatible tmc driver (IOIN version field)
    uint32_t IOIN = ReadRegister(params, Registers::IOIN);
    if (((IOIN >> 24) != 0x11) | !(IOIN & (1 << 6))) ///if the version is incorrect or an always 1 bit is 0 (the supposed SD_MODE pin that doesn't exist on this driver variant)
        return true; // @todo return some kind of failure

    ///clear reset_flag as we are (re)initializing
    errorFlags.reset_flag = false;

    ///apply chopper parameters
    uint32_t chopconf = 0;
    chopconf |= (uint32_t)(3 & 0x0F) << 0; //toff
    chopconf |= (uint32_t)(5 & 0x07) << 4; //hstrt
    chopconf |= (uint32_t)(1 & 0x0F) << 7; //hend
    chopconf |= (uint32_t)(2 & 0x03) << 15; //tbl
    chopconf |= (uint32_t)(currents.vSense & 0x01) << 17; //vsense
    chopconf |= (uint32_t)(params.uSteps & 0x0F) << 24; //mres
    chopconf |= (uint32_t)((bool)params.uSteps) << 28; //intpol
    chopconf |= (uint32_t)(1 & 0x01) << 29; //dedge
    WriteRegister(params, Registers::CHOPCONF, chopconf);

    ///apply currents
    SetCurrents(params, currents);

    ///instant powerdown ramp
    WriteRegister(params, Registers::TPOWERDOWN, 0);

    ///Stallguard parameters
    int8_t sg_thrs = 3; // @todo 7bit two's complement for the sg_thrs
    WriteRegister(params, Registers::COOLCONF, (((uint32_t)sg_thrs) << 16)); // @todo should be configurable
    WriteRegister(params, Registers::TCOOLTHRS, 400); // @todo should be configurable

    ///Write stealth mode config and setup diag0 output
    uint32_t gconf = 0;
    gconf |= (uint32_t)(1 & 0x01) << 2; //en_pwm_mode - always enabled since we can control it's effect with TPWMTHRS (0=only stealthchop, 0xFFFFF=only spreadcycle)
    gconf |= (uint32_t)(1 & 0x01) << 7; //diag0_stall - diag0 is open collector => active low with external pullups
    WriteRegister(params, Registers::GCONF, gconf);

    ///stealthChop parameters
    uint32_t pwmconf = 0; /// @todo All of these parameters should be configurable
    pwmconf |= (uint32_t)(240 & 0xFF) << 0; //PWM_AMPL
    pwmconf |= (uint32_t)(4 & 0xFF) << 8; //PWM_GRAD
    pwmconf |= (uint32_t)(2 & 0x03) << 16; //pwm_freq
    pwmconf |= (uint32_t)(1 & 0x01) << 18; //pwm_autoscale
    WriteRegister(params, Registers::PWMCONF, pwmconf);

    ///TPWMTHRS: switching velocity between stealthChop and spreadCycle. Stallguard is also disabled if the velocity falls below this. Should be set as high as possible when homing.
    SetMode(params, mode);
    return false;
}

void TMC2130::SetMode(const MotorParams &params, MotorMode mode) {
    this->mode = mode;

    ///0xFFF00 is used as a "Normal" mode threshold since stealthchop will be used at standstill.
    WriteRegister(params, Registers::TPWMTHRS, (mode == Stealth) ? 70 : 0xFFF00); // @todo should be configurable
}

void TMC2130::SetCurrents(const MotorParams &params, const MotorCurrents &currents) {
    this->currents = currents;

    uint32_t ihold_irun = 0;
    ihold_irun |= (uint32_t)(currents.iHold & 0x1F) << 0; //ihold
    ihold_irun |= (uint32_t)(currents.iRun & 0x1F) << 8; //irun
    ihold_irun |= (uint32_t)(15 & 0x0F) << 16; //IHOLDDELAY
    WriteRegister(params, Registers::IHOLD_IRUN, ihold_irun);
}

void TMC2130::SetEnabled(const MotorParams &params, bool enabled) {
    hal::shr16::shr16.SetTMCDir(params.idx, enabled);
    this->enabled = enabled;
}

uint32_t TMC2130::ReadRegister(const MotorParams &params, Registers reg) {
    uint8_t pData[5] = { (uint8_t)reg };
    _spi_tx_rx(params, pData);
    pData[0] = 0;
    _spi_tx_rx(params, pData);
    _handle_spi_status(params, pData[0]);
    return ((uint32_t)pData[1] << 24 | (uint32_t)pData[2] << 16 | (uint32_t)pData[3] << 8 | (uint32_t)pData[4]);
}

void TMC2130::WriteRegister(const MotorParams &params, Registers reg, uint32_t data) {
    uint8_t pData[5] = { (uint8_t)((uint8_t)(reg) | 0x80), (uint8_t)(data >> 24), (uint8_t)(data >> 16), (uint8_t)(data >> 8), (uint8_t)data };
    _spi_tx_rx(params, pData);
    _handle_spi_status(params, pData[0]);
}

void TMC2130::_spi_tx_rx(const MotorParams &params, uint8_t (&pData)[5]) {
    hal::gpio::WritePin(params.csPin, hal::gpio::Level::low);
    for (uint8_t i = 0; i < sizeof(pData); i++)
        pData[i] = hal::spi::TxRx(params.spi, pData[i]);
    hal::gpio::WritePin(params.csPin, hal::gpio::Level::high);
}

void TMC2130::_handle_spi_status(const MotorParams &params, uint8_t status) {
    errorFlags.reset_flag |= status & (1 << 0);
    errorFlags.driver_error |= status & (1 << 1);
}

} // namespace tmc2130
} // namespace hal
