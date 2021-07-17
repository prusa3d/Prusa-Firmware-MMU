#pragma once
#include "../hal/gpio.h"
#include "../hal/shr16.h"

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
    uint8_t idx; ///< SHR16 index
    bool dirOn; ///< forward direction
    gpio::GPIO_pin csPin; ///< CS pin
    gpio::GPIO_pin stepPin; ///< step pin
    gpio::GPIO_pin sgPin; ///< stallguard pin
    uint8_t uSteps; ///< microstep resolution
};

struct MotorCurrents {
    bool vSense; ///< VSense current scaling
    uint8_t iRun; ///< Running current
    uint8_t iHold; ///< Holding current
};

class TMC2130 {
    MotorMode mode;
    MotorCurrents currents;

public:
    /// Constructor
    TMC2130(const MotorParams &params,
        const MotorCurrents &currents,
        MotorMode mode);

    /// (re)initialization of the chip
    void Init(const MotorParams &params);

    /// Get the current motor mode
    MotorMode Mode() const {
        return mode;
    }

    /// Set the current motor mode
    void SetMode(MotorMode mode);

    /// Get the current motor currents
    const MotorCurrents &Currents() const {
        return currents;
    }

    /// Set the current motor currents
    void SetCurrents(const MotorCurrents &currents);

    /// Return enabled state (TODO)
    static bool Enabled(const MotorParams &params);

    /// Enable/Disable the motor
    static void SetEnabled(const MotorParams &params, bool enabled) {
        hal::shr16::shr16.SetTMCDir(params.idx, enabled);
    }

    /// Set direction
    static inline void SetDir(const MotorParams &params, bool dir) {
        hal::shr16::shr16.SetTMCDir(params.idx, dir ^ params.dirOn);
    }

    /// Step the motor
    static inline void Step(const MotorParams &params) {
        gpio::TogglePin(params.stepPin); // assumes DEDGE
    }

    /// Return SG state
    static inline bool Stall(const MotorParams &params) {
        return gpio::ReadPin(params.sgPin) == gpio::Level::high;
    }

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
};

} // namespace tmc2130
} // namespace hal
