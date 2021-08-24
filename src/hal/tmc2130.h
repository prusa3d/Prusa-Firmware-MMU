#pragma once
#include "../config/config.h"
#include "../hal/gpio.h"
#include "../hal/shr16.h"
#include "../hal/spi.h"

namespace hal {

/// TMC2130 interface
/// There are multiple TMC2130 on our board, so there will be multiple
/// instances of this class
/// @@TODO @leptun - design some lightweight TMC2130 interface
namespace tmc2130 {

enum MotorMode : uint8_t {
    Stealth,
    Normal
};

struct MotorParams {
    const hal::spi::SPI_TypeDef *spi;
    uint8_t idx; ///< SHR16 index
    bool dirOn; ///< forward direction
    gpio::GPIO_pin csPin; ///< CS pin
    gpio::GPIO_pin stepPin; ///< step pin
    gpio::GPIO_pin sgPin; ///< stallguard pin
    config::MRes mRes; ///< microstep resolution
};

struct MotorCurrents {
    bool vSense; ///< VSense current scaling
    uint8_t iRun; ///< Running current
    uint8_t iHold; ///< Holding current
};

struct __attribute__((packed)) ErrorFlags {
    uint8_t reset_flag : 1; ///< driver restarted
    uint8_t uv_cp : 1; ///< undervoltage on charge pump
    uint8_t s2g : 1; ///< short to ground
    uint8_t otpw : 1; ///< over temperature pre-warning
    uint8_t ot : 1; ///< over temperature hard
    inline ErrorFlags()
        : reset_flag(0)
        , uv_cp(0)
        , s2g(0)
        , otpw(0)
        , ot(0) {}
};

/// TMC2130 interface - instances of this class are hidden in modules::motion::Motion::AxisData
class TMC2130 {
public:
    /// TMC2130 register addresses
    enum class Registers : uint8_t {
        /// General Configuration Registers
        GCONF = 0x00,
        GSTAT = 0x01,
        IOIN = 0x04,

        /// Velocity Dependent Driver Feature Control Register Set
        IHOLD_IRUN = 0x10,
        TPOWERDOWN = 0x11,
        TSTEP = 0x12,
        TPWMTHRS = 0x13,
        TCOOLTHRS = 0x14,
        THIGH = 0x15,

        /// Motor Driver Registers
        MSCNT = 0x6A,
        CHOPCONF = 0x6C,
        COOLCONF = 0x6D,
        DRV_STATUS = 0x6F,
        PWMCONF = 0x70,
    };

    /// Constructor
    TMC2130() = default;

    /// (re)initialization of the chip - please note this is necessary due to some HW flaws in the original MMU boards.
    /// And yes, the TMC may not get correctly initialized.
    /// @returns true if the TMC2130 was inited correctly
    bool Init(const MotorParams &params,
        const MotorCurrents &currents,
        MotorMode mode);

    /// Set the current motor mode
    void SetMode(const MotorParams &params, MotorMode mode);

    /// Set the current motor currents
    void SetCurrents(const MotorParams &params, const MotorCurrents &currents);

    /// Return enabled state
    const bool Enabled() const {
        return enabled;
    }

    /// Enable/Disable the motor
    void SetEnabled(const MotorParams &params, bool enabled);

    /// Set direction
    static inline void SetDir(const MotorParams &params, bool dir) {
        hal::shr16::shr16.SetTMCDir(params.idx, dir ^ params.dirOn);
    }

    /// Step the motor
    static inline void Step(const MotorParams &params) {
        gpio::TogglePin(params.stepPin); // assumes DEDGE
    }

    /// Return SG state
    static inline bool SampleDiag(const MotorParams &params) {
        return gpio::ReadPin(params.sgPin) == gpio::Level::low;
    }

    inline bool Stalled() const {
        return sg_counter == 0;
    }

    void ClearStallguard(const MotorParams &params);

    /// Should be called periodically from main loop. Maybe not all the time. Once every 10 ms is probably enough
    bool CheckForErrors(const MotorParams &params);

    inline ErrorFlags GetErrorFlags() const {
        return errorFlags;
    }

    /// Reads a driver register and updates the status flags
    uint32_t ReadRegister(const MotorParams &params, Registers reg);

    /// Writes a driver register and updates the status flags
    void WriteRegister(const MotorParams &params, Registers reg, uint32_t data);

    /// Used for polling the DIAG pin. Should be called from the stepper isr periodically when moving.
    void Isr(const MotorParams &params);

private:
    void _spi_tx_rx(const MotorParams &params, uint8_t (&pData)[5]);
    void _handle_spi_status(const MotorParams &params, uint8_t status);

    ErrorFlags errorFlags;
    bool enabled = false;
    uint8_t sg_counter;
};

} // namespace tmc2130
} // namespace hal
