#ifndef CONSUME_PRODUCE_H
#define CONSUME_PRODUCE_H

#include "circular_buffer.h"

#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>

/// Structure to store essential things for between thread communication
typedef struct consume_produce
{
    pthread_mutex_t mutex_buffer;
    sem_t sem_empty;
    sem_t sem_filled;
    circular_buffer buffer;
} consume_produce;

/// Initialize consume_produce
/// cp_destroy() should be called to free resources
///
/// CP is consume_produce to initialize, BUF is size of buffer, ITEM is size of each item in buffer
///
/// returns 0 if successful, -1 if failed
int cp_init(consume_produce *cp, size_t buf_size, size_t item_size);

/// Destroy consume_produce
///
/// CP is consume_produce to destroy
///
/// returns 0 if successful, -1 if failed
int cp_destroy(consume_produce *cp);

/// Produce to consume_produce
///
/// CP is to produce to, ITEM is element to produce
///
/// returns 0 if successful, -1 if failed
int cp_produce(consume_produce *cp, const void *item);

/// Consume from consume_produce
///
/// CP is to consume from, ITEM used to return
///
/// returns 0 if successful, -1 if failed
int cp_consume(consume_produce *cp, void *item);

#endif
