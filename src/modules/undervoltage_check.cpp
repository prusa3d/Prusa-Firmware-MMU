/// @file undervoltage_check.cpp
#include "undervoltage_check.h"
#include "../hal/adc.h"
#include "../logic/error_codes.h"
#include "../panic.h"

namespace modules {
namespace undervoltage_check {

Undervoltage_check uv_vcc;

void Undervoltage_check::Step() {
    uint16_t vcc_val;

    // dummy reads are so that the final measurement is valid
    for (uint8_t i = 0; i < config::VCCADCReadCnt; i++) {
        vcc_val = hal::adc::ReadADC(config::VCCADCIndex);
    }

    if (vcc_val > config::VCCADCThreshold) {
        Panic(ErrorCode::MCU_UNDERVOLTAGE_VCC);
    }
}

} // namespace undervoltage_check
} // namespace modules
