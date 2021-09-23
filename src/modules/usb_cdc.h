#pragma once
#include <stdint.h>

namespace modules {

/// The usb namespace provides all necessary facilities related to the USB interface.
namespace usb {

class CDC {
public:
    constexpr inline CDC() {}

    void Init();

    void Step();

private:
};

/// The one and only instance of Selector in the FW
extern CDC cdc;

} // namespace usb
} // namespace modules

namespace mu = modules::usb;
