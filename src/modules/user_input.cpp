/// @file user_input.cpp
#include "user_input.h"
#include "buttons.h"

namespace modules {
namespace user_input {

UserInput userInput;

void UserInput::Step() {
    if (buttons::buttons.ButtonPressed(mb::Left))
        eventQueue.push(Event::Left);
    if (buttons::buttons.ButtonPressed(mb::Middle))
        eventQueue.push(Event::Middle);
    if (buttons::buttons.ButtonPressed(mb::Right))
        eventQueue.push(Event::Right);
}

void UserInput::ProcessMessage(uint8_t ev) {
    eventQueue.push((Event)(ev | Event::FromPrinter));
}

Event UserInput::StripFromPrinterBit(uint8_t e) {
    e &= (uint8_t)(~Event::FromPrinter);
    return (Event)e;
}

Event UserInput::ConsumeEvent() {
    if (printerInCharge) {
        Event rv = eventQueue.front();
        if (rv & Event::FromPrinter) {
            eventQueue.pop(rv);
            return StripFromPrinterBit(rv);
        }
        return Event::NoEvent;
    } else {
        Event rv;
        eventQueue.pop(rv);
        return StripFromPrinterBit(rv);
    }
}

Event UserInput::ConsumeEventForPrinter() {
    if (eventQueue.empty())
        return Event::NoEvent;
    Event rv = eventQueue.front();
    if (rv & Event::FromPrinter) {
        // do not send the same buttons back but leave them in the queue for the MMU FW
        return Event::NoEvent;
    }
    eventQueue.pop(rv);
    return StripFromPrinterBit(rv);
}

void UserInput::Clear() {
    while (!eventQueue.empty()) {
        Event x;
        eventQueue.pop(x);
    }
}

} // namespace user_input
} // namespace modules
