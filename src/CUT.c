#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// Using enum for better code readability in the future
typedef enum Thread
{
    Reader,   // 0
    Analyzer, // 1
    Printer,  // 2
    Watchdog, // 3
    Logger    // 4
} Thread;

#define THREAD_NUM 5 // Reader, Analyzer, Printer, Watchdog, Logger

char stat_cpu[1024];

/// I SHOULD READ USAGE FOR EACH CPU!
/// I CAN USE GLOBAL variable/struct TO SHARE DATA BETWEEN THREADS

void *open_proc_stat()
{
    printf("Opening /proc/stat file...\n");

    FILE *file = fopen("/proc/stat", "r");
    if (!file)
    {
        perror("Could not open stat file");
        exit(EXIT_FAILURE);
    }
    printf("Opened /proc/stat file\n");
    return file;
}

void *thread_Reader()
{
    /// Every time i want to read new stats, I NEED TO REOPEN /proc/stat file!
    /// Maybe I should implement opening file and reading from file as ONE function? (and write unit test for it)

    /// While loop
    /// 1. Open file
    /// 2. Read from file
    /// 3. Send Raw/Extracted data to another thread
    /// 4. repeat

    /// TODO: BUFFER "cpu" objects extracted from /proc/stat
    /// TODO: Implement Producer-Consumer (DO NOT USE spinlocks)

    printf("Reader thread started\n");
    /// TODO: CHANGE IT TO WHILE LOOP
    for (int i = 0; i < 10; i++)
    {
        printf("Iteration No-%d\n", i);

        // Open file
        FILE *pFile = open_proc_stat();

        // Read every line from file which starts with "cpu"
        while (1)
        {
            char str[224];
            // Read the line and compare if it starts with "cpu"
            if ((fgets(str, 224, pFile) == NULL) | (strncmp(str, "cpu", (size_t)3) != 0))
            {
                // Does not start with "cpu" -> break loop (because it is last "cpu" line)
                // printf("fgets() == NULL or strncmp() != 0");
                break;
            }
            // Starts with "cpu" -> continue
            printf("%s", str);

            /// TODO: Send string to Analyzer
        }
        printf("\n");

        // Close file and sleep for some time
        fclose(pFile);
        sleep(1);
    }
    printf("Reader thread closes\n");
    return NULL;
}

int main()
{
    pthread_t thread[THREAD_NUM];

    pthread_create(&thread[Reader], NULL, &thread_Reader, NULL);

    pthread_join(thread[Reader], NULL);

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
