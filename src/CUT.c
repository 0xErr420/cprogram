#include "utils.h"
#include "reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

// Using enum for better code readability in the future
enum Thread
{
    Reader = 1, // 1
    Analyzer,   // 2
    Printer,    // 3
    Watchdog,   // 4
    Logger      // 5
};

/// Array of threads identifiers should be 5 elements long
/// [Reader, Analyzer, Printer, Watchdog, Logger]
#define THREAD_NUM 5

/// Size of buffer for Reader-Analyzer
#define RA_BUFF_SIZE 10

/// ==== Communication between threads ====
ConsumeProduce_type reader_analyzer;  // Reader-Analyzer
ConsumeProduce_type analyzer_printer; // Analyzer-Printer

/// ==== Aruments for threads ====
ArgsThread_type args_reader;   // Reader args
ArgsThread_type args_analyzer; // Analyzer args
ArgsThread_type args_printer;  // Printer args

void *thread_Producer(void *arg)
{
    ArgsThread_type *args = (ArgsThread_type *)(arg);
    while (1)
    {
        // Produce
        FILE *proc = open_proc_stat();
        while (1)
        {
            char str[CPU_READ_SIZE];
            // Read the line and compare if it starts with "cpu"
            if ((fgets(str, 224, proc) == NULL) | (strncmp(str, "cpu", (size_t)3) != 0))
            {
                // Does not start with "cpu" -> break loop (previous line was last "cpu" line)
                // printf("fgets() == NULL or strncmp() != 0");
                break;
            }
            // Starts with "cpu" -> continue

            sem_wait(&args->arg2->sem_empty); // Wait for empty
            pthread_mutex_lock(&args->arg2->mutex_buffer);

            if (cb_push_back(&args->arg2->buffer, str) != 0)
                perror("Failed to add element to circular buffer");
            printf("Produced: %s", str);

            pthread_mutex_unlock(&args->arg2->mutex_buffer);
            sem_post(&args->arg2->sem_filled); // Tell other thread there is filled available
        }
        sleep(1);
    }
}

/// Calculate CPU usage (in percentages) for each CPU core from /proc/stat.
void *thread_Analyzer(void *arg)
{
    /// Consumer for RA_buffer:
    /// 1. Wait for filled element in buffer (increments "filled" semaphore)
    /// 2. Lock mutex
    /// 3. Do some calculation
    /// 4. Unlock mutex
    /// 5. Decrement "empty" semaphore

    ArgsThread_type *args = (ArgsThread_type *)(arg);

    while (1)
    {
        // Consume
        sem_wait(&args->arg1->sem_filled); // Wait for filled
        pthread_mutex_lock(&args->arg1->mutex_buffer);

        char str[CPU_READ_SIZE];
        if (cb_pop_front(&args->arg1->buffer, str) != 0)
            perror("Failed to get element from circular buffer");
        printf("Consumed: %s\n", str);

        pthread_mutex_unlock(&args->arg1->mutex_buffer);
        sem_post(&args->arg1->sem_empty); // Tell other thread there is empty available
        sleep(2);
    }
}

int main()
{
    // Array of thread identifiers
    pthread_t thread[THREAD_NUM];

    /// Initialization
    /// -------------------------------
    /// for Read-Analyzer
    pthread_mutex_init(&reader_analyzer.mutex_buffer, NULL);
    sem_init(&reader_analyzer.sem_empty, 0, RA_BUFF_SIZE);
    sem_init(&reader_analyzer.sem_filled, 0, 0);
    cb_init(&reader_analyzer.buffer, RA_BUFF_SIZE, CPU_READ_SIZE);

    // `args_reader->arg1` stays None because this thread doesn't have consumer
    args_reader.arg2 = &reader_analyzer;
    args_analyzer.arg1 = &reader_analyzer;

    /// for Analyzer-Printer
    //
    /// -------------------------------

    /// Start threads
    /// -------------------------------
    // if (pthread_create(&thread[Reader], NULL, &thread_Reader, NULL) != 0)
    // {
    //     perror("Failed to create Reader thread");
    // }

    /// REMOVE: Remove this thread in the future (it is just a test for producer role)
    pthread_create(&thread[Reader], NULL, &thread_Producer, &args_reader);

    if (pthread_create(&thread[Analyzer], NULL, &thread_Analyzer, &args_analyzer) != 0)
    {
        perror("Failed to create Analyzer thread");
    }
    /// -------------------------------

    /// Manage signaling
    /// -------------------------------
    /// here
    /// -------------------------------

    /// Close threads
    /// -------------------------------
    /// REMOVE: Remove this in the future
    if (pthread_join(thread[1], NULL) != 0)
    {
        perror("Failed to join thread 1");
    }
    if (pthread_join(thread[Reader], NULL) != 0)
    {
        perror("Failed to join thread");
    }
    /// -------------------------------

    /// Destroy
    /// -------------------------------
    cb_free(&reader_analyzer.buffer);
    sem_destroy(&reader_analyzer.sem_empty);
    sem_destroy(&reader_analyzer.sem_filled);
    pthread_mutex_destroy(&reader_analyzer.mutex_buffer);
    /// -------------------------------

    // printf("Print stat_cpu:\n%s\n", stat_cpu);

    // // Extract values with formated string
    // unsigned long long user = 0ULL, nice = 0, system = 0, idle = 0;
    // unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;
    // sscanf(stat_cpu,
    //        "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
    //        &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);

    // printf("Print extracted values:\n%-6s %-16s %-16s %-16s %-16s %-16s %-16s %-16s %-16s %-16s %-16s\n",
    //        "Data:", "user", "nice", "system", "idle", "iowait", "irq", "softirq", "steal", "guest", "guestnice");

    // printf("%-6s %-16llu %-16llu %-16llu %-16llu %-16llu %-16llu %-16llu %-16llu %-16llu %-16llu\n",
    //        "cpu", user, nice, system, idle, iowait, irq, softirq, steal, guest, guestnice);

    return EXIT_SUCCESS;
}
