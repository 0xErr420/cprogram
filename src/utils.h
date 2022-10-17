#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>

/// Structure for cpu fields
typedef struct proc_stat
{
    char cpu[6]; // name of 'cpu' (For example: cpu, cpu0, cpu1, etc.)

    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long guest;
    unsigned long long guest_nice;
} ProcStat_type;

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
} CircularBuffer_type;

/// Structure to store essential things for between thread communication
typedef struct consume_produce
{
    pthread_mutex_t mutex_buffer;
    sem_t sem_empty;
    sem_t sem_filled;
    CircularBuffer_type buffer;
} ConsumeProduce_type;

/// Structure to pass multiple arguments to thread
///
/// arg1 - consumer, arg2 - producer
typedef struct args_thread
{
    ConsumeProduce_type *arg1; // Consumer
    ConsumeProduce_type *arg2; // Producer
} ArgsThread_type;

/// Initialize circular buffer.
/// Dynamically allocates memory, it is necessary to call the function `cb_free(...)`.
///
/// CAPACITY is size of buffer, SZ is size of individual elements
///
/// returns 0 if successful, -1 if failed
int cb_init(CircularBuffer_type *cb, size_t capacity, size_t sz);

/// Free dynamically allocated memory for circular buffer
void cb_free(CircularBuffer_type *cb);

/// Add element to queue
///
/// ITEM is element to add
///
/// returns 0 if successful, -1 if failed
int cb_push_back(CircularBuffer_type *cb, const void *item);

/// Get element from queue
///
/// ITEM used to return value
///
/// returns 0 if successful, -1 if failed
int cb_pop_front(CircularBuffer_type *cb, void *item);

#endif
