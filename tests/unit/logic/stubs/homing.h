#pragma once

namespace logic {
class CommandBase;
}

void SimulateIdlerHoming(logic::CommandBase &cb);
void SimulateSelectorHoming(logic::CommandBase &cb, bool waitForParkedPosition = false);
void SimulateIdlerAndSelectorHoming(logic::CommandBase &cb);
bool SimulateFailedHomeFirstTime(logic::CommandBase &cb);
bool SimulateFailedHomeSelectorRepeated(logic::CommandBase &cb);
