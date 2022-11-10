#include "printer.h"
#include "utils.h"
#include "group.h"
#include "circular_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/// Add corresponding percentages from cur to sum
///
/// returns 0 if successful, -1 if failed
static int sum_cpu_percentages(group *sum, group *cur);

/// Divide each sum with div to get average percentage
///
/// returns 0 if successful, -1 if failed
static int avg_cpu_percentages(group *sum_percentages, int div);

/// Print percentages
///
/// returns 0 if successful, -1 if failed
static int print_percentages(group *avg_percentages);

void *thread_Printer(void *arg)
{
    ArgsThread_type *args = (ArgsThread_type *)(arg);
    while (1) // Thread loop
    {
        // Group used to store summed percentages
        group g_sum_percentages;
        int count_divide = 1;

        // Initialize group with first element in buffer
        sem_wait(&args->arg1->sem_filled); // Wait for filled
        pthread_mutex_lock(&args->arg1->mutex_buffer);

        if (cb_pop_front(&args->arg1->buffer, &g_sum_percentages) != 0)
            perror("Failed to get element from circular buffer");

        pthread_mutex_unlock(&args->arg1->mutex_buffer);
        sem_post(&args->arg1->sem_empty); // Tell other thread there is empty available

        // Get count of how many items are available in buffer
        int filled_count;
        if (sem_getvalue(&args->arg1->sem_filled, &filled_count) != 0)
        {
            /// TODO: Signal to watchdog about this problem (-1 for example, would mean thread failed)
            perror("Failed to get value of semaphore sem_filled");
            /// TODO: Properly exit on error
            break; // Exit thread loop
        }

        // Loop for each available group in buffer and sum percentages for each cpu
        for (int i = 0; i < filled_count; i++)
        {
            // Current group with percentages
            group g_cur_percentages;

            // Consume group from buffer
            sem_wait(&args->arg1->sem_filled); // Wait for filled
            pthread_mutex_lock(&args->arg1->mutex_buffer);

            if (cb_pop_front(&args->arg1->buffer, &g_cur_percentages) != 0)
                perror("Failed to get element from circular buffer");

            pthread_mutex_unlock(&args->arg1->mutex_buffer);
            sem_post(&args->arg1->sem_empty); // Tell other thread there is empty available

            // Add corresponding percentages to sum_percentages
            if (sum_cpu_percentages(&g_sum_percentages, &g_cur_percentages) != 0)
            {
                perror("Failed to sum cpu percentages");
                /// TODO: Properly exit on error
                break; // Exit for loop
            }

            // Increment counter
            count_divide++;

            group_free(&g_cur_percentages);
        } // End of for loop

        // Calculate average percentages
        if (avg_cpu_percentages(&g_sum_percentages, count_divide) != 0)
        {
            perror("Failed to average cpu percentages");
            /// TODO: Properly exit on error
            break;
        }

        printf("\n============================ CPU usage ============================\n%7s: ", "Usage");
        // Print percentages
        if (print_percentages(&g_sum_percentages) != 0)
        {
            perror("Failed to print percentages");
            /// TODO: Properly exit on error
            break;
        }
        printf("\n");

        group_free(&g_sum_percentages);

        // Test if there was a signal to exit
        pthread_testcancel();

        sleep(1);
    } // End of thread loop

    return NULL;
}

static int sum_cpu_percentages(group *sum_percentages, group *cur_percentages)
{
    for (int i = 0; i < sum_percentages->count; i++)
    {
        cpu_usage sum_perc;
        cpu_usage cur_perc;

        if (group_get(sum_percentages, &sum_perc, i) != 0 || group_get(cur_percentages, &cur_perc, i) != 0)
        {
            return -1;
        }

        sum_perc.percentage += cur_perc.percentage;

        if (group_set(sum_percentages, &sum_perc, i) != 0)
        {
            return -1;
        }
    }
    return 0;
}

static int avg_cpu_percentages(group *sum_percentages, int div)
{
    for (int i = 0; i < sum_percentages->count; i++)
    {
        cpu_usage sum;

        if (group_get(sum_percentages, &sum, i) != 0)
        {
            return -1;
        }

        sum.percentage /= div;

        if (group_set(sum_percentages, &sum, i) != 0)
        {
            return -1;
        }
    }
    return 0;
}

static int print_percentages(group *avg_percentages)
{
    for (int i = 0; i < avg_percentages->count; i++)
    {
        cpu_usage perc;

        if (group_get(avg_percentages, &perc, i) != 0)
        {
            return -1;
        }

        printf("cpu[%d]: %9f%%    ", perc.cpu_id, perc.percentage);
    }
    return 0;
}