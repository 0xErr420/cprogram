#include "reader.h"
#include "utils.h"
#include "circular_buffer.h"
#include "group.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void *thread_Reader(void *arg)
{
    /// TODO: remove any printf(), check for return values, handle errors
    ArgsThread_type *args = (ArgsThread_type *)(arg);
    while (1) // Thread loop
    {
        // Get available number of cpus
        long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
        if (num_cpus == -1)
        {
            /// TODO: Signal to watchdog about this problem (-1 for example, would mean thread failed)
            perror("Failed to call sysconf(_SC_NPROCESSORS_ONLN) to get available CPUs");
            break; // Exit thread loop
        }

        // Wait for empty and then post it back
        // (In other words, produce only after empty producer slot appeared)
        sem_wait(&args->arg2->sem_empty); // Wait for empty
        sem_post(&args->arg2->sem_empty); // Post empty

        // Initialize 'g_cpus' group for cpu_fields
        group g_cpus;
        if (group_init(&g_cpus, num_cpus + 1, sizeof(cpu_fields)) == -1)
        {
            /// TODO: Signal to watchdog about this problem (-1 for example, would mean thread failed)
            perror("Failed to initialize group g_cpus");
            break; // Exit thread loop
        }

        /// ==== Produce into buffer ====
        // Open file
        FILE *stat_file = fopen("/proc/stat", "r");
        if (stat_file == NULL)
        {
            /// TODO: Signal to watchdog about this problem (-1 for example, would mean thread failed)
            perror("Failed to open /proc/stat file");
            break; // Exit thread loop
        }

        // Read file and store extracted cpus into group
        while (1)
        {
            char str[220];  // Buffer used for reading line from file
            cpu_fields cpu; // Strcucture used for extracted cpu fields

            // Read the line
            if (fgets(str, sizeof(str), stat_file) == NULL)
            {
                // man fgets(): If the End-of-File is encountered and no characters have been read, a null pointer is returned
                perror("Failed to read /proc/stat file");
                break; // Exit read loop
            }
            // Compare if line starts with "cpu"
            if (strncmp(str, "cpu", (size_t)3) != 0)
            {
                // Does not start with "cpu" -> break loop (previous line was last "cpu" line)
                break; // Exit read loop
            }
            // Line starts with "cpu" -> continue

            // Extract cpu fields into struct
            if (strncmp(str, "cpu ", (size_t)4) == 0) // if "cpu_" has space instead of digit
            {
                cpu.cpu_id = -1; // it is total cpu
                // extract fields of cpu
                int r = sscanf(str,
                               "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                               &cpu.user, &cpu.nice, &cpu.system, &cpu.idle, &cpu.iowait, &cpu.irq, &cpu.softirq, &cpu.steal, &cpu.guest, &cpu.guest_nice);
                if (r < 4)
                {
                    fprintf(stderr, "Failed to extract cpu fields, sscanf() returned %d, minimum 4 expected", r);
                    break; // Exit read loop
                }
            }
            else // if "cpu_" has digit
            {
                // extract id of cpu
                int r = sscanf(str, "cpu%d ", &cpu.cpu_id);
                if (r <= 0)
                {
                    fprintf(stderr, "Failed to read cpu id field, sscanf() returned %d, 1 expected", r);
                    break; // Exit read loop
                }
                // extract fields of cpu
                r = sscanf(str,
                           "cpu%*d  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                           &cpu.user, &cpu.nice, &cpu.system, &cpu.idle, &cpu.iowait, &cpu.irq, &cpu.softirq, &cpu.steal, &cpu.guest, &cpu.guest_nice);
                if (r < 4)
                {
                    fprintf(stderr, "Failed to extract cpu fields, sscanf() returned %d, minimum 4 expected", r);
                    break; // Exit read loop
                }
            }

            // Add cpu to group
            if (group_push(&g_cpus, &cpu) == -1)
            {
                perror("Failed to push element to g_cpus group");
                break; // Exit read loop
            }

        } // End of read file loop
        fclose(stat_file);

        // Send to another thread
        sem_wait(&args->arg2->sem_empty); // Wait for empty
        pthread_mutex_lock(&args->arg2->mutex_buffer);

        if (cb_push_back(&args->arg2->buffer, &g_cpus) != 0)
            perror("Failed to add element to circular buffer");

        pthread_mutex_unlock(&args->arg2->mutex_buffer);
        sem_post(&args->arg2->sem_filled); // Tell other thread there is filled available

        // Test if there was a signal to exit
        pthread_testcancel();

    } // End of thread loop

    /// TODO: In case of thread exit, free all produced elements stored in buffer

    return NULL;
}
