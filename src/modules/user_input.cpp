#include "user_input.h"
#include "buttons.h"

namespace modules {
namespace user_input {

UserInput userInput;

void UserInput::Step() {
    if (buttons::buttons.ButtonPressed(0))
        eventQueue.push_back(Event::Left);
    if (buttons::buttons.ButtonPressed(1))
        eventQueue.push_back(Event::Middle);
    if (buttons::buttons.ButtonPressed(2))
        eventQueue.push_back(Event::Right);
}

void UserInput::ProcessMessage(uint8_t ev) {
    eventQueue.push_back((Event)ev);
}

Event UserInput::ConsumeEvent() {
    if (eventQueue.IsEmpty())
        return Event::NoEvent;
    Event rv;
    eventQueue.ConsumeFirst(rv);
    return rv;
}

void UserInput::Clear() {
    while (!eventQueue.IsEmpty()) {
        Event x;
        eventQueue.ConsumeFirst(x);
    }
}

} // namespace user_input
} // namespace modules
