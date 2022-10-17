#include "analyzer.h"
#include "utils.h"
#include "reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/// Analyzer thread calculates CPU usage (in percentages) for each CPU core from /proc/stat.
void *thread_Analyzer(void *arg)
{
    /// 1. Read 'cpu' lines from buffer
    /// 2. Analyze and calculate percentages
    /// 3. Send data to Printer
    /// 4. repeat

    /// TODO: remove any printf(), check for return values, handle errors
    ArgsThread_type *args = (ArgsThread_type *)(arg);
    // int bool_prev = 0; // Was there a previous?
    while (1)
    {
        /// Consume:
        sem_wait(&args->arg1->sem_filled); // Wait for filled
        pthread_mutex_lock(&args->arg1->mutex_buffer);

        char str[CPU_READ_SIZE];
        if (cb_pop_front(&args->arg1->buffer, str) != 0)
            perror("Failed to get element from circular buffer");

        /// I NEED to group 'cpu's into one struct

        printf("Consumed: %s\n", str);
        pthread_mutex_unlock(&args->arg1->mutex_buffer);
        sem_post(&args->arg1->sem_empty); // Tell other thread there is empty available

        /// TODO: remove sleep
        sleep(1);

        /// TODO: Produce:

        // Test if there was a signal to exit
        pthread_testcancel();
    }
    return NULL;
}
