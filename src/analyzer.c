#include "analyzer.h"
#include "utils.h"
#include "group.h"
#include "circular_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void *thread_Analyzer(void *arg)
{
    /// TODO: remove any printf(), check for return values, handle errors
    ArgsThread_type *args = (ArgsThread_type *)(arg);
    while (1) // Thread loop
    {
        // Wait for empty and then post it back
        // (In other words, consume and produce only after empty producer slot appeared)
        sem_wait(&args->arg2->sem_empty); // Wait for empty
        sem_post(&args->arg2->sem_empty); // Post empty

        /// ==== Consume from buffer ====

        // Group for cpu_fields to consume group from buffer
        group g_cpus;

        // Receive group from buffer
        sem_wait(&args->arg1->sem_filled); // Wait for filled
        pthread_mutex_lock(&args->arg1->mutex_buffer);

        if (cb_pop_front(&args->arg1->buffer, &g_cpus) != 0)
            perror("Failed to get element from circular buffer");

        pthread_mutex_unlock(&args->arg1->mutex_buffer);
        sem_post(&args->arg1->sem_empty); // Tell other thread there is empty available

        // Initialize 'cpu_percentages' group for cpu_usage
        group g_percentages;
        if (group_init(&g_percentages, g_cpus.count, sizeof(cpu_usage)) == -1)
        {
            /// TODO: Signal to watchdog about this problem (-1 for example, would mean thread failed)
            perror("Failed to initialize group g_percentages");
            break; // Exit thread loop
        }

        /// ==== Produce into buffer ====
        // Loop for each cpu in 'g_cpus' group, calculate percentage and add to 'g_percentages' group
        for (int i = 0; i < g_cpus.count; i++)
        {
            cpu_fields cpu;           // Strcucture used to pop cpu from group
            cpu_usage cpu_percentage; // Structure used to store calculated percentages

            // Pop cpu from group
            if (group_pop(&g_cpus, &cpu) == -1)
            {
                perror("Failed to pop element from g_cpus group");
                break; // Exit for loop
            }

            // Simplified version of percentage calculation
            unsigned long long idle = cpu.idle + cpu.iowait;
            unsigned long long non_idle = cpu.user + cpu.nice + cpu.system + cpu.irq + cpu.softirq + cpu.steal;

            double total = idle + non_idle;
            double usage_percent = (non_idle * 100) / total;

            cpu_percentage.cpu_id = cpu.cpu_id;
            cpu_percentage.percentage = usage_percent;

            // Add percentage to group
            if (group_push(&g_percentages, &cpu_percentage) == -1)
            {
                perror("Failed to push element to g_percentages group");
                break; // Exit for loop
            }
        } // End of for loop

        // Free resources of consumed 'g_cpus' group
        group_free(&g_cpus);

        // Send to another thread
        sem_wait(&args->arg2->sem_empty); // Wait for empty
        pthread_mutex_lock(&args->arg2->mutex_buffer);

        if (cb_push_back(&args->arg2->buffer, &g_percentages) != 0)
            perror("Failed to add element to circular buffer");

        pthread_mutex_unlock(&args->arg2->mutex_buffer);
        sem_post(&args->arg2->sem_filled); // Tell other thread there is filled available

        // Test if there was a signal to exit
        pthread_testcancel();

        // /// Calculate percentage for current cpu
        // unsigned long long PrevIdle = prev_idle + prev_iowait;
        // unsigned long long PrevNonIdle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;

        // unsigned long long Idle = idle + iowait;
        // unsigned long long NonIdle = user + nice + system + irq + softirq + steal;

        // unsigned long long PrevTotal = PrevIdle + PrevNonIdle;
        // unsigned long long Total = Idle + NonIdle;

        // double TotalD = Total = PrevTotal;
        // double IdleD = Idle - PrevIdle;

        // double CpuPercentage = (TotalD - IdleD) / TotalD;

    } // End of thread loop

    /// TODO: In case of thread exit, free all produced elements stored in buffer

    return NULL;
}
