#include "reader.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void *thread_Reader(void *arg)
{
    /// 1. Read 'cpu' lines
    /// 2. Send each line to Analyzer
    /// 3. repeat

    /// TODO: remove any printf(), check for return values, handle errors
    ArgsThread_type *args = (ArgsThread_type *)(arg);
    while (1)
    {
        /// Produce:
        FILE *proc = open_proc_stat();
        while (1)
        {
            char str[CPU_READ_SIZE];
            // Read the line and compare if it starts with "cpu"
            if ((fgets(str, CPU_READ_SIZE, proc) == NULL) | (strncmp(str, "cpu", (size_t)3) != 0))
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

        /// TODO: remove sleep
        sleep(1);
    }
    return NULL;
}

void *open_proc_stat()
{
    /// TODO: refactor, remove printf(), handle errors and return value
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

void *read_proc_stat(FILE *file)
{
    /// TODO: file read loop
    /// 1. Open file
    /// 2. Read from file loop:
    ///     2.1 Read line
    ///     2.2 If line starts with "cpu" -> continue, else -> break loop (go to point 4)
    ///     2.3 Send line to buffer for another thread
    /// 4. Close file
    /// 5. repeat
}
