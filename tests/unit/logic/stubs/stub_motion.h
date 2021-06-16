#pragma once
#include <stdint.h>

namespace modules {
namespace motion {

struct AxisSim {
    int32_t pos;
    int32_t targetPos;
    bool enabled;
    bool homed;
    bool stallGuard;
};

extern AxisSim axes[3];

} // namespace motion
} // namespace modules
