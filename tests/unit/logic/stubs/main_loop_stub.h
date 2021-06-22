#pragma once
#include "../../../../src/logic/command_base.h"

extern void main_loop();
extern void ForceReinitAllAutomata();

template <typename SM, typename COND>
bool WhileCondition(SM &sm, COND cond, uint32_t maxLoops = 5000) {
    while (cond(maxLoops) && --maxLoops) {
        main_loop();
        sm.Step();
    }
    return maxLoops > 0;
}

template <typename SM>
bool WhileTopState(SM &sm, ProgressCode state, uint32_t maxLoops = 5000) {
    return WhileCondition(
        sm, [&](int) { return sm.TopLevelState() == state; }, maxLoops);
}
