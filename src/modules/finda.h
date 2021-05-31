#pragma once
#include <stdint.h>

namespace modules {
namespace finda {

enum { On,
    Off };

class FINDA {
public:
    inline FINDA() = default;
    uint8_t Status() const;
    void Step();
};

extern FINDA finda;

} // namespace finda
} // namespace modules
