#pragma once
#include "../../../../src/logic/command_base.h"

extern void main_loop();
extern void ForceReinitAllAutomata();

extern logic::CommandBase *currentCommand;

template <typename COND>
bool WhileCondition(COND cond, uint32_t maxLoops = 5000) {
    while (cond() && --maxLoops) {
        main_loop();
    }
    return maxLoops > 0;
}
