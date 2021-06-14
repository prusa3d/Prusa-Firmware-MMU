#pragma once
#include <stdint.h>

namespace modules {
namespace globals {

class Globals {
public:
    void Init();

    uint8_t ActiveSlot() const;
    void SetActiveSlot(uint8_t newActiveSlot);

    bool FilamentLoaded() const;
    void SetFilamentLoaded(bool newFilamentLoaded);

private:
    uint8_t activeSlot;
    bool filamentLoaded;
};

extern Globals globals;

} // namespace globals
} // namespace modules
