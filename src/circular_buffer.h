#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H
#include <stddef.h>

/// Structure to store circular buffer
typedef struct circular_buffer
{
    void *buffer;     // data buffer
    void *buffer_end; // end of data buffer
    size_t capacity;  // maximum number of items in the buffer
    size_t count;     // number of items in the buffer
    size_t sz;        // size of each item in the buffer
    void *head;       // pointer to head
    void *tail;       // pointer to tail
} circular_buffer;

/// Initialize circular buffer
/// Dynamically allocates memory, it is necessary to call the function `cb_free(...)`
///
/// CAPACITY is size of buffer, SZ is size of individual elements
///
/// returns 0 if successful, -1 if failed
int cb_init(circular_buffer *cb, size_t capacity, size_t sz);

/// Free dynamically allocated memory for circular buffer
void cb_free(circular_buffer *cb);

/// Push element to queue
///
/// ITEM is element to add
///
/// returns 0 if successful, -1 if failed
int cb_push_back(circular_buffer *cb, const void *item);

/// Pop element from queue
///
/// ITEM used to return value
///
/// returns 0 if successful, -1 if failed
int cb_pop_front(circular_buffer *cb, void *item);

#endif
