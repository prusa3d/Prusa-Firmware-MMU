#pragma once
#include <stdint.h>
#include "../hal/circle_buffer.hpp"

namespace modules {

/// User input module collects input from buttons and from communication for the logic layer
namespace user_input {

enum class Event : int8_t {
    NoEvent = -1,
    Left = 0,
    Middle = 1,
    Right = 2
};

class UserInput {

public:
    UserInput() = default;

    /// collects the buttons' state and enqueues the corresponding event
    void Step();

    /// enqueues a user event coming from a communication
    void ProcessMessage(uint8_t ev);

    /// dequeues the most recent event from the queue for processing
    Event ConsumeEvent();

    /// @returns true if there is at least one event in the event queue
    bool AnyEvent() const { return !eventQueue.IsEmpty(); }

    /// Remove all buffered events from the event queue
    void Clear();

private:
    CircleBuffer<Event, 4> eventQueue;
};

extern UserInput userInput;

} // namespace user_input
} // namespace modules
