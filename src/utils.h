#ifndef UTILS_H
#define UTILS_H
#include "circular_buffer.h"

#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>

/// Structure for cpu fields
/// (all fields of 'cpu' line)
typedef struct cpu_fields
{
    int cpu_id; // Digit of cpu (cpu0, cpu1, etc), -1 if total cpu (without a digit)
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
} cpu_fields;

/// Structure to store cpu percentage
typedef struct cpu_usage
{
    int cpu_id;
    double percentage;
} cpu_usage;

/// Structure to store essential things for between thread communication
typedef struct consume_produce
{
    pthread_mutex_t mutex_buffer;
    sem_t sem_empty;
    sem_t sem_filled;
    circular_buffer buffer;
} ConsumeProduce_type;

/// Structure to pass multiple arguments to thread
///
/// arg1 - consumer, arg2 - producer
typedef struct args_thread
{
    ConsumeProduce_type *arg1; // Consumer
    ConsumeProduce_type *arg2; // Producer
} ArgsThread_type;

#endif
