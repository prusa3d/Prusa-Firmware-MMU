#pragma once

/// TMC2130 interface
/// There are multiple TMC2130 on our board, so there will be multiple instances of this class
/// @@TODO @leptun - design some lightweight TMC2130 interface

namespace hal {

class TMC2130 {

public:
    /// constructor - do some one-time init stuff here
    TMC2130(uint8_t id);
    /// (re)initialization of the chip
    void Init();
};

} // namespace hal
