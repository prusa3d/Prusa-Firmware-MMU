#pragma once

#include <stdint.h>
#include <stddef.h>

/// A generic circular index class which can be used to build circular buffers
/// Can hold up to (size-1) elements
/// @param index_t data type of indices into array of elements
///   (recommended to keep uint8_fast8_t as single byte operations are atomical on the AVR)
/// @param size number of index positions + 1.
///   It is recommended to keep a power of 2 to allow for optimal code generation on the AVR (there is no HW modulo instruction)
template <typename index_t = uint_fast8_t, size_t size = 16>
class CircularIndex {
public:
    constexpr inline CircularIndex()
        : tail(0)
        , head(0) {}

    /// @returns true if empty
    inline bool empty() const {
        return tail == head;
    }

    /// @returns true if full
    inline bool full() const {
        return next(head) == tail;
    }

    /// Advance the head index of the buffer.
    /// No checks are performed. full() needs to be queried beforehand.
    inline void push() {
        head = next(head);
    }

    /// Advance the tail index from the buffer.
    /// No checks are performed. empty() needs to be queried beforehand.
    inline void pop() {
        tail = next(tail);
    }

    /// @returns return the tail index from the buffer.
    /// Does not perform any range checks for performance reasons, should be preceeded by if(!empty()) in the user code
    inline index_t front() const {
        return tail;
    }

    /// @returns return the head index from the buffer.
    /// Does not perform any range checks for performance reasons, should be preceeded by if(!empty()) in the user code
    inline index_t back() const {
        return head;
    }

protected:
    index_t tail; ///< index of element to read (pop/extract) from the buffer
    index_t head; ///< index of an empty spot or element insertion (write)

    /// @returns next index wrapped past the end of the array of elements
    static index_t next(index_t index) { return (index + 1) % size; }
};

/// A generic circular buffer class
/// Can hold up to (size-1) elements
/// @param T data type of stored elements
/// @param index_t data type of indices into array of elements
///   (recommended to keep uint8_fast8_t as single byte operations are atomical on the AVR)
/// @param size number of elements to store + 1.
///   It is recommended to keep a power of 2 to allow for optimal code generation on the AVR (there is no HW modulo instruction)
template <typename T = uint8_t, typename index_t = uint_fast8_t, size_t size = 16>
class CircularBuffer {
public:
    inline bool empty() const {
        return index.empty();
    }

    bool full() const {
        return index.full();
    }

    /// Insert an element into the buffer.
    /// Checks for empty spot for the element and does not change the buffer content
    /// in case the buffer is full.
    /// @returns true if the insertion was successful (i.e. there was an empty spot for the element)
    bool push(T elem) {
        if (full())
            return false;
        data[index.back()] = elem;
        index.push();
        return true;
    }

    /// @returns peeks the current element to extract from the buffer, however the element is left in the buffer
    /// Does not perform any range checks for performance reasons, should be preceeded by if(!empty()) in the user code
    inline T front() const {
        return data[index.front()];
    }

    /// Extracts the current element from the buffer
    /// @returns true in case there was an element for extraction (i.e. the buffer was not empty)
    bool pop(T &elem) {
        if (empty())
            return false;
        elem = front();
        index.pop();
        return true;
    }

protected:
    T data[size]; ///< array of stored elements
    CircularIndex<index_t, size> index; ///< circular index
};
