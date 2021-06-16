#pragma once

namespace modules {
namespace motion {

struct AxisSim {
    uint32_t pos;
    uint32_t targetPos;
    bool enabled;
    bool homed;
    bool stallGuard;
};

extern AxisSim axes[3];

} // namespace motion
} // namespace modules
