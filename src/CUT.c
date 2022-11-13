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

/// Free groups inside consume_produce buffer
///
/// CP is to free from
static void free_groups(consume_produce *cp);

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
    /// ==== Signaling ====
    struct sigaction sact;
    memset(&sact, 0, sizeof(struct sigaction));
    sact.sa_handler = handle_sigterm;
    sigaction(SIGINT, &sact, NULL);  // Handle Interrupt signal (Ctrl+C by user)
    sigaction(SIGQUIT, &sact, NULL); // Handle Quit signal (Ctrl+\ by user)

    /// ==== Initialization ====
    if (cp_init(&reader_analyzer, RA_BUFF_SIZE, sizeof(group)) != 0 || cp_init(&analyzer_printer, AP_BUFF_SIZE, sizeof(group)) != 0) // buffers store groups
        return -1;

    args_reader.arg2 = &reader_analyzer; // arg1 stays undefined
    args_analyzer.arg1 = &reader_analyzer;
    args_analyzer.arg2 = &analyzer_printer;
    args_printer.arg1 = &analyzer_printer; // arg2 stays undefined

    /// ==== Start threads ====
    pthread_t thread[THREAD_NUM]; // Array of thread identifiers

    if (pthread_create(&thread[Reader], NULL, &thread_Reader, &args_reader) != 0 ||
        pthread_create(&thread[Analyzer], NULL, &thread_Analyzer, &args_analyzer) != 0 ||
        pthread_create(&thread[Printer], NULL, &thread_Printer, &args_printer) != 0)
    {
        perror("Failed to create thread");
        cp_destroy(&reader_analyzer);
        cp_destroy(&analyzer_printer);
        return -1;
    }

    /// ==== Manage signaling ====
    while (!sig_to_quit)
    {
        unsigned int t = sleep(3);
        if (t > 0)
        {
            printf(" <<< Interrupted with %d sec to go, finishing...\n", t);
        }
    }

    /// ==== Close threads ====
    if (pthread_cancel(thread[Reader]) != 0 ||
        pthread_cancel(thread[Analyzer]) != 0 ||
        pthread_cancel(thread[Printer]) != 0)
    {
        perror("Failed to cancel thread");
        cp_destroy(&reader_analyzer);
        cp_destroy(&analyzer_printer);
        return -1;
    }

    /// ==== Join threads ====
    for (int i = 1; i <= Printer; i++)
    {
        if (pthread_join(thread[i], NULL) != 0)
        {
            perror("Failed to join thread");
            cp_destroy(&reader_analyzer);
            cp_destroy(&analyzer_printer);
            return -1;
        }
    }

    /// ==== Destroy ====
    free_groups(&reader_analyzer);
    free_groups(&analyzer_printer);

    if (cp_destroy(&reader_analyzer) != 0 || cp_destroy(&analyzer_printer) != 0)
    {
        perror("Failed to destroy");
        return -1;
    }

    printf("Exiting\n");

    return 0;
}

static void free_groups(consume_produce *cp)
{
    for (size_t i = 0; i < cp->buffer.capacity; i++) // Free allocated groups
    {
        group grp;
        if (cb_pop_front(&cp->buffer, &grp) != 0)
            break;
        group_free(&grp);
    }
}
