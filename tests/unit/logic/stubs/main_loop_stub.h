#pragma once
#include "../../../../src/logic/command_base.h"

extern void main_loop();
extern void ForceReinitAllAutomata();

template <typename SM, typename COND>
bool WhileCondition(SM &sm, COND cond, uint32_t maxLoops = 5000) {
    uint32_t step = 0;
    while (cond(step) && --maxLoops) {
        main_loop();
        sm.Step();
        ++step;
    }
    return maxLoops > 0;
}

template <typename SM>
bool WhileTopState(SM &sm, ProgressCode state, uint32_t maxLoops = 5000) {
    return WhileCondition(
        sm, [&](int) { return sm.TopLevelState() == state; }, maxLoops);
}

extern void EnsureActiveSlotIndex(uint8_t slot);

extern void SetFINDAStateAndDebounce(bool press);

// these are recommended max steps for simulated movement of the idler and selector
// - roughly the amount of motion steps from one end to the other + some margin
// ... could be computed in the future from the pre-set number of microsteps and real positions
static constexpr uint32_t idlerEngageDisengageMaxSteps = 40000UL;
static constexpr uint32_t selectorMoveMaxSteps = 40000UL;
