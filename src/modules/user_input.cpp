#include "user_input.h"
#include "buttons.h"

namespace modules {
namespace user_input {

UserInput userInput;

void UserInput::Step() {
    if (buttons::buttons.ButtonPressed(0))
        eventQueue.push(Event::Left);
    if (buttons::buttons.ButtonPressed(1))
        eventQueue.push(Event::Middle);
    if (buttons::buttons.ButtonPressed(2))
        eventQueue.push(Event::Right);
}

void UserInput::ProcessMessage(uint8_t ev) {
    eventQueue.push((Event)ev);
}

Event UserInput::ConsumeEvent() {
    if (eventQueue.empty())
        return Event::NoEvent;
    Event rv;
    eventQueue.pop(rv);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    return rv;
#pragma GCC diagnostic pop
}

void UserInput::Clear() {
    while (!eventQueue.empty()) {
        Event x;
        eventQueue.pop(x);
    }
}

} // namespace user_input
} // namespace modules
