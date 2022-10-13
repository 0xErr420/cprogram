#include "utils.h"

#include <stdlib.h>
#include <string.h>

int cb_init(circular_buffer *cb, size_t capacity, size_t sz)
{
    cb->buffer = malloc(capacity * sz);
    if (cb->buffer == NULL)
    {
        // handle error
        return -1;
    }
    cb->buffer_end = (char *)cb->buffer + capacity * sz;
    cb->capacity = capacity;
    cb->count = 0;
    cb->sz = sz;
    cb->head = cb->buffer;
    cb->tail = cb->buffer;
    return 0;
}

void cb_free(circular_buffer *cb)
{
    free(cb->buffer);
    // clear out other fields too, just to be safe
}

int cb_push_back(circular_buffer *cb, const void *item)
{
    if (cb->count == cb->capacity)
    {
        // handle error
        return -1;
    }
    memcpy(cb->head, item, cb->sz);
    cb->head = (char *)cb->head + cb->sz;
    if (cb->head == cb->buffer_end)
        cb->head = cb->buffer;
    cb->count++;
    return 0;
}

int cb_pop_front(circular_buffer *cb, void *item)
{
    if (cb->count == 0)
    {
        // handle error
        return -1;
    }
    memcpy(item, cb->tail, cb->sz);
    cb->tail = (char *)cb->tail + cb->sz;
    if (cb->tail == cb->buffer_end)
        cb->tail = cb->buffer;
    cb->count--;
    return 0;
}
