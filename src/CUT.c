#include "reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

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
#define THREAD_NUM (5) // Reader, Analyzer, Printer, Watchdog, Logger

int main()
{
    // Array of thread identifiers
    pthread_t thread[THREAD_NUM];

    // Start threads
    if (pthread_create(&thread[Reader], NULL, &thread_Reader, NULL) != 0)
    {
        perror("Failed to create thread");
    }

    // Close threads
    if (pthread_join(thread[Reader], NULL) != 0)
    {
        perror("Failed to join thread");
    }

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
