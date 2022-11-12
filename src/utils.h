#ifndef UTILS_H
#define UTILS_H
#include "consume_produce.h"

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

/// Structure to pass multiple arguments to thread
///
/// arg1 - consumer, arg2 - producer
typedef struct args_thread
{
    consume_produce *arg1; // Consumer
    consume_produce *arg2; // Producer
} args_thread;

#endif
