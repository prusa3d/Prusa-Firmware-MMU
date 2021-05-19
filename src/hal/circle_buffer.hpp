// circle_buffer.hpp
#pragma once

#include <stdint.h>
#include <stddef.h>
/*****************************************************************************/
// general circular buffer
// you can never use entire size
// because write position (end) cannot be equal to begin
// because begin == end == empty
template <class T, size_t SIZE>
class CircleBuffer {
public:
    using Elem = T;

protected:
    T data[SIZE];
    volatile size_t begin; // position of first element
    volatile size_t end; // position behind last element == write position
    volatile size_t pushed;
    static void incrementIndex(volatile size_t &index) { index = (index + 1) % SIZE; }
    static void decrementIndex(volatile size_t &index) { index = (index + SIZE - 1) % SIZE; }

public:
    CircleBuffer()
        : begin(0)
        , end(0)
        , pushed(0) {}

    void push_back(T elem);
    bool push_back_DontRewrite(T elem);
    size_t Count() const { return (end + SIZE - begin) % SIZE; }
    bool IsEmpty() const { return begin == end; }
    bool CanPush() const {
        size_t index = begin;
        incrementIndex(index);
        return (index != end);
    }
    size_t PushedCount() const { return pushed; }

    constexpr size_t Size() const { return SIZE; }

    bool ConsumeFirst(T &elem); // data must be processed before next push_back
    bool ConsumeLast(T &elem); // data must be processed before next push_back
    const T &GetFirstIfAble() const; // data must be processed before next push_back, must not be empty
    const T &GetLastIfAble() const; // data must be processed before next push_back, must not be empty
};

template <class T, size_t SIZE>
void CircleBuffer<T, SIZE>::push_back(T elem) {
    data[end] = elem;
    incrementIndex(end);
    if (begin == end) { //begin just was erased, set new begin
        incrementIndex(begin);
    }
    ++pushed;
}

template <class T, size_t SIZE>
bool CircleBuffer<T, SIZE>::push_back_DontRewrite(T elem) {
    size_t index = begin;
    incrementIndex(index);
    if (index != end) {
        push_back(elem);
        return true;
    }
    return false;
}

template <class T, size_t SIZE>
bool CircleBuffer<T, SIZE>::ConsumeFirst(T &elem) {
    if (IsEmpty())
        return false;
    elem = GetFirstIfAble();
    incrementIndex(begin);
    return true;
}

template <class T, size_t SIZE>
bool CircleBuffer<T, SIZE>::ConsumeLast(T &elem) {
    if (IsEmpty())
        return false;
    elem = GetLastIfAble();
    decrementIndex(end);
    return true;
}

template <class T, size_t SIZE>
const T &CircleBuffer<T, SIZE>::GetFirstIfAble() const {
    return data[begin];
}

template <class T, size_t SIZE>
const T &CircleBuffer<T, SIZE>::GetLastIfAble() const {
    size_t index = end;
    decrementIndex(index);
    return data[index];
}
