#include "utils.h"
#include "group.h"
#include "reader.h"
#include "analyzer.h"
#include "printer.h"

#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
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
/// Size of buffer for Analyzer-Printer
#define AP_BUFF_SIZE 10

/// ==== Communication between threads ====
static consume_produce reader_analyzer;  // Reader-Analyzer
static consume_produce analyzer_printer; // Analyzer-Printer

/// ==== Arguments for threads ====
static args_thread args_reader;   // Reader args
static args_thread args_analyzer; // Analyzer args
static args_thread args_printer;  // Printer args

/// ==== Signaling ====
static volatile sig_atomic_t sig_to_quit = 0;

static void handle_sigterm(int signum)
{
    sig_to_quit = signum;
}

int main()
{
    // Array of thread identifiers
    pthread_t thread[THREAD_NUM];

    /// ==== Signaling ====
    struct sigaction sact;
    memset(&sact, 0, sizeof(struct sigaction));
    sact.sa_handler = handle_sigterm;
    sigaction(SIGINT, &sact, NULL);  // Handle Interrupt signal (Ctrl+C by user)
    sigaction(SIGQUIT, &sact, NULL); // Handle Quit signal (Ctrl+\ by user)

    /// ==== Initialization ====
    /// for Read-Analyzer
    pthread_mutex_init(&reader_analyzer.mutex_buffer, NULL);
    sem_init(&reader_analyzer.sem_empty, 0, RA_BUFF_SIZE);
    sem_init(&reader_analyzer.sem_filled, 0, 0);
    cb_init(&reader_analyzer.buffer, RA_BUFF_SIZE, sizeof(group)); // Buffer stores array of 'group' structures
    // `args_reader->arg1` stays undefined because this thread only produces
    args_reader.arg2 = &reader_analyzer;
    args_analyzer.arg1 = &reader_analyzer;
    /// for Analyzer-Printer
    pthread_mutex_init(&analyzer_printer.mutex_buffer, NULL);
    sem_init(&analyzer_printer.sem_empty, 0, AP_BUFF_SIZE);
    sem_init(&analyzer_printer.sem_filled, 0, 0);
    cb_init(&analyzer_printer.buffer, AP_BUFF_SIZE, sizeof(group)); // Buffer stores array of 'group' structures
    args_analyzer.arg2 = &analyzer_printer;
    args_printer.arg1 = &analyzer_printer;
    // `args_printer->arg2` stays undefined because this thread only consumes
    /// -------------------------------

    /// ==== Start threads ====
    if (pthread_create(&thread[Reader], NULL, &thread_Reader, &args_reader) != 0)
    {
        perror("Failed to create Reader thread");
    }
    if (pthread_create(&thread[Analyzer], NULL, &thread_Analyzer, &args_analyzer) != 0)
    {
        perror("Failed to create Analyzer thread");
    }
    if (pthread_create(&thread[Printer], NULL, &thread_Printer, &args_printer) != 0)
    {
        perror("Failed to create Printer thread");
    }
    /// -------------------------------

    /// ==== Manage signaling ====
    while (!sig_to_quit)
    {
        unsigned int t = sleep(3);
        if (t > 0)
        {
            printf(" <<< Interrupted with %d sec to go, finishing...\n", t);
        }
    }
    /// -------------------------------

    /// ==== Close threads ====
    if (pthread_cancel(thread[Reader]) != 0)
    {
        perror("Failed to cancel Reader thread");
    }
    if (pthread_cancel(thread[Analyzer]) != 0)
    {
        perror("Failed to cancel Analyzer thread");
    }
    if (pthread_cancel(thread[Printer]) != 0)
    {
        perror("Failed to cancel Printer thread");
    }
    /// -------------------------------

    /// ==== Join threads ====
    for (int i = 1; i <= 3; i++)
    {
        if (pthread_join(thread[i], NULL) != 0)
        {
            perror("Failed to join thread");
        }
    }
    /// -------------------------------

    /// ==== Free resources and Destroy ====
    /// for Read-Analyzer
    for (size_t i = 0; i < reader_analyzer.buffer.capacity; i++) // Free allocated groups
    {
        group g_group;
        if (cb_pop_front(&reader_analyzer.buffer, &g_group) != 0)
        {
            fprintf(stderr, "Destroy reader-analyzer: Failed to get element from circular buffer\n");
            break;
        }
        group_free(&g_group);
    }
    cb_free(&reader_analyzer.buffer);
    sem_destroy(&reader_analyzer.sem_empty);
    sem_destroy(&reader_analyzer.sem_filled);
    pthread_mutex_destroy(&reader_analyzer.mutex_buffer);
    /// for Analyzer-Printer
    for (size_t i = 0; i < analyzer_printer.buffer.capacity; i++) // Free allocated groups
    {
        group g_group;
        if (cb_pop_front(&analyzer_printer.buffer, &g_group) != 0)
        {
            fprintf(stderr, "Destroy analyzer-printer: Failed to get element from circular buffer\n");
            break;
        }
        group_free(&g_group);
    }
    cb_free(&analyzer_printer.buffer);
    sem_destroy(&analyzer_printer.sem_empty);
    sem_destroy(&analyzer_printer.sem_filled);
    pthread_mutex_destroy(&analyzer_printer.mutex_buffer);
    /// -------------------------------

    printf("Exiting\n");
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

    return 0;
}
