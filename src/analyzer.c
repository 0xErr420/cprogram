#include "analyzer.h"
#include "utils.h"
#include "consume_produce.h"
#include "group.h"
#include "circular_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/// Calculate usage percentage for cpu
static void calc_percentage(cpu_fields *cpu, cpu_usage *perc);

void *thread_Analyzer(void *arg)
{
    args_thread *args = (args_thread *)(arg);
    while (1) // Thread loop
    {
        // Wait for empty and then post it back (consume and produce only after empty producer slot appeared)
        sem_wait(&args->arg2->sem_empty);
        sem_post(&args->arg2->sem_empty);

        // Group used to consume from buffer
        group g_cpus;
        // Group used to produce to buffer
        group g_percentages;

        // Consume group with cpu fields
        if (cp_consume(args->arg1, &g_cpus) != 0)
        {
            perror("Failed to consume");
            return NULL;
        }

        // Initialize group for cpu percentages
        if (group_init(&g_percentages, g_cpus.count, sizeof(cpu_usage)) != 0)
        {
            /// TODO: Signal to watchdog
            perror("Failed to initialize group");
            group_free(&g_cpus);
            return NULL;
        }

        // Loop for each cpu in group, calculate percentage and add to percentages group
        for (size_t i = 0; i < g_cpus.count; i++)
        {
            cpu_fields cpu;           // Strcucture used to pop cpu from group
            cpu_usage cpu_percentage; // Structure used to store calculated percentages

            // Pop cpu from group
            if (group_pop(&g_cpus, &cpu) != 0)
            {
                perror("Failed to pop element from group");
                group_free(&g_percentages);
                group_free(&g_cpus);
                return NULL;
            }
            // Calculate percentage
            calc_percentage(&cpu, &cpu_percentage);

            // Add percentage to group
            if (group_push(&g_percentages, &cpu_percentage) == -1)
            {
                perror("Failed to push element to group");
                group_free(&g_percentages);
                group_free(&g_cpus);
                return NULL;
            }
        }
        // Free resources of consumed group
        group_free(&g_cpus);

        // Send to another thread
        if (cp_produce(args->arg2, &g_percentages) != 0)
        {
            perror("Failed to produce");
            group_free(&g_percentages);
            return NULL;
        }

        // Test if there was a signal to exit
        pthread_testcancel();

    } // End of thread loop
    return NULL;
}

static void calc_percentage(cpu_fields *cpu, cpu_usage *perc)
{
    // Simplified version of percentage calculation
    double idle = cpu->idle + cpu->iowait;
    double non_idle = cpu->user + cpu->nice + cpu->system + cpu->irq + cpu->softirq + cpu->steal;

    double total = idle + non_idle;
    double usage_percent = (non_idle * 100) / total;

    perc->cpu_id = cpu->cpu_id;
    perc->percentage = usage_percent;

    // /// Calculate percentage for cpu
    // unsigned long long PrevIdle = prev_idle + prev_iowait;
    // unsigned long long PrevNonIdle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;

    // unsigned long long Idle = idle + iowait;
    // unsigned long long NonIdle = user + nice + system + irq + softirq + steal;

    // unsigned long long PrevTotal = PrevIdle + PrevNonIdle;
    // unsigned long long Total = Idle + NonIdle;

    // double TotalD = Total = PrevTotal;
    // double IdleD = Idle - PrevIdle;

    // double CpuPercentage = (TotalD - IdleD) / TotalD;
}
