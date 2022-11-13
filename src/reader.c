#include "reader.h"
#include "utils.h"
#include "consume_produce.h"
#include "circular_buffer.h"
#include "group.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/// Extract cpu field from string
///
/// STR is to extract from, CPU used to return
///
/// returns 0 if successful, -1 if failed
static int extract_cpu_fields(const char *str, cpu_fields *cpu);

void *thread_Reader(void *arg)
{
    args_thread *args = (args_thread *)(arg);

    // Get available number of cpus
    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cpus == -1)
    {
        /// TODO: Signal to watchdog
        perror("Failed to call sysconf(_SC_NPROCESSORS_ONLN) to get available CPUs");
        return NULL;
    }

    while (1) // Thread loop
    {
        // Wait for empty and then post it back (produce only after empty producer slot appeared)
        sem_wait(&args->arg2->sem_empty);
        sem_post(&args->arg2->sem_empty);

        // Initialize group for cpu_fields
        group g_cpus;
        if (group_init(&g_cpus, num_cpus + 1, sizeof(cpu_fields)) != 0)
        {
            /// TODO: Signal to watchdog
            perror("Failed to initialize group");
            return NULL;
        }

        // Open file
        FILE *stat_file = fopen("/proc/stat", "r");
        if (stat_file == NULL)
        {
            /// TODO: Signal to watchdog
            perror("Failed to open /proc/stat file");
            group_free(&g_cpus);
            return NULL;
        }

        // Read file and store extracted cpus into group
        while (1)
        {
            char str[220];  // Buffer used for reading line from file
            cpu_fields cpu; // Strcucture used for extracted cpu fields

            // Read the line
            if (fgets(str, sizeof(str), stat_file) == NULL)
            {
                // man fgets(): If the End-of-File or Error occurs, a null pointer is returned. Use feof() or ferror() to determine if it was EOF or an error
                perror("Failed to read /proc/stat file");
                fclose(stat_file);
                group_free(&g_cpus);
                return NULL;
            }
            // Compare if line starts with "cpu"
            if (strncmp(str, "cpu", (size_t)3) != 0)
                break; // Does not start with "cpu" -> break loop (previous line was last "cpu" line)

            // Line starts with "cpu" -> continue

            // Extract cpu fields
            if (extract_cpu_fields(str, &cpu) != 0)
            {
                perror("Failed to extract cpu fields");
                fclose(stat_file);
                group_free(&g_cpus);
                return NULL;
            }
            // Add cpu to group
            if (group_push(&g_cpus, &cpu) == -1)
            {
                perror("Failed to push element to group");
                fclose(stat_file);
                group_free(&g_cpus);
                return NULL;
            }
        }
        fclose(stat_file);

        // Send to another thread
        if (cp_produce(args->arg2, &g_cpus) != 0)
        {
            perror("Failed to produce");
            group_free(&g_cpus);
            return NULL;
        }

        // Test if there was a signal to exit
        pthread_testcancel();

    } // End of thread loop
    return NULL;
}

static int extract_cpu_fields(const char *str, cpu_fields *cpu)
{
    if (strncmp(str, "cpu ", (size_t)4) == 0) // if "cpu_" has space instead of digit
    {
        cpu->cpu_id = -1; // it is total cpu
        // extract fields of cpu
        int r = sscanf(str,
                       "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                       &cpu->user, &cpu->nice, &cpu->system, &cpu->idle, &cpu->iowait, &cpu->irq, &cpu->softirq, &cpu->steal, &cpu->guest, &cpu->guest_nice);
        if (r < 4)
        {
            // fprintf(stderr, "Failed to extract cpu fields, sscanf() returned %d, minimum 4 expected", r);
            return -1;
        }
    }
    else // if "cpu_" has digit
    {
        // extract id of cpu
        int r = sscanf(str, "cpu%d ", &cpu->cpu_id);
        if (r <= 0)
        {
            // fprintf(stderr, "Failed to read cpu id field, sscanf() returned %d, 1 expected", r);
            return -1;
        }
        // extract fields of cpu
        r = sscanf(str,
                   "cpu%*d  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &cpu->user, &cpu->nice, &cpu->system, &cpu->idle, &cpu->iowait, &cpu->irq, &cpu->softirq, &cpu->steal, &cpu->guest, &cpu->guest_nice);
        if (r < 4)
        {
            // fprintf(stderr, "Failed to extract cpu fields, sscanf() returned %d, minimum 4 expected", r);
            return -1;
        }
    }
    return 0;
}
