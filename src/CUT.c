#include "utils.h"
#include "reader.h"
#include "analyzer.h"

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
#define AP_BUFF_SIZE 0

/// ==== Communication between threads ====
static ConsumeProduce_type reader_analyzer;  // Reader-Analyzer
static ConsumeProduce_type analyzer_printer; // Analyzer-Printer

/// ==== Aruments for threads ====
static ArgsThread_type args_reader;   // Reader args
static ArgsThread_type args_analyzer; // Analyzer args
static ArgsThread_type args_printer;  // Printer args

/// ==== Signaling ====
static volatile sig_atomic_t sig_quit = 0;

static void handle_sigterm(int signum)
{
    sig_quit = 1;
    printf("   <<< Signal!\n");
}

/// Printer thread
void *thread_Printer(void *args)
{
    return NULL;
}

int main()
{
    // Array of thread identifiers
    pthread_t thread[THREAD_NUM];

    /// ==== Signaling ====
    struct sigaction sact;
    memset(&sact, 0, sizeof(struct sigaction));
    sact.sa_handler = handle_sigterm;
    sigaction(SIGINT, &sact, NULL);

    /// ==== Initialization ====
    /// -------------------------------
    /// for Read-Analyzer
    pthread_mutex_init(&reader_analyzer.mutex_buffer, NULL);
    sem_init(&reader_analyzer.sem_empty, 0, RA_BUFF_SIZE);
    sem_init(&reader_analyzer.sem_filled, 0, 0);
    cb_init(&reader_analyzer.buffer, RA_BUFF_SIZE, CPU_READ_SIZE);
    /// `args_reader->arg1` stays None because this thread doesn't have consumer
    args_reader.arg2 = &reader_analyzer;
    args_analyzer.arg1 = &reader_analyzer;

    /// for Analyzer-Printer
    // pthread_mutex_init(&analyzer_printer.mutex_buffer, NULL);
    // sem_init(&analyzer_printer.sem_empty, 0, AP_BUFF_SIZE);
    // sem_init(&analyzer_printer.sem_filled, 0, 0);
    // cb_init(&analyzer_printer.buffer, AP_BUFF_SIZE, ); /// Stopped here!
    /// -------------------------------

    /// ==== Start threads ====
    /// -------------------------------
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

    /// TODO: refactor, remove printf()
    printf("Waiting for signal...\n");
    /// ==== Manage signaling ====
    while (!sig_quit)
    {
        unsigned int t = sleep(3);
        if (t > 0)
        {
            printf("Interrupted with %d sec to go, finishing...\n", t);
        }
    }
    /// -------------------------------

    /// Close threads
    /// -------------------------------
    printf("Closing threads...\n");

    if (pthread_cancel(thread[Reader]) != 0)
    {
        perror("Failed to cancel Reader thread");
    }
    if (pthread_cancel(thread[Analyzer]) != 0)
    {
        perror("Failed to cancel Analyzer thread");
    }

    /// TODO: change to THREAD_NUM
    for (int i = 1; i <= 3; i++)
    {
        if (pthread_join(thread[i], NULL) != 0)
        {
            perror("Failed to join thread");
        }
    }
    // if (pthread_join(thread[Reader], NULL) != 0)
    // {
    //     perror("Failed to join Reader thread");
    // }
    // if (pthread_join(thread[Analyzer], NULL) != 0)
    // {
    //     perror("Failed to join Analyzer thread");
    // }
    /// -------------------------------

    /// Destroy
    /// -------------------------------
    cb_free(&reader_analyzer.buffer);
    sem_destroy(&reader_analyzer.sem_empty);
    sem_destroy(&reader_analyzer.sem_filled);
    pthread_mutex_destroy(&reader_analyzer.mutex_buffer);
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
