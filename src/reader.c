#include "reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void *thread_Reader()
{
    /// Every time i want to read new stats, I NEED TO REOPEN /proc/stat file!
    /// Maybe I should implement opening file and reading from file as ONE function? (and write unit test for it)

    /// TODO: loop:
    /// 1. Read cpu stats
    /// 2. Send data to Analyzer
    /// 3. repeat

    /// TODO: BUFFER "cpu" objects extracted from /proc/stat
    /// TODO: Implement Producer-Consumer (DO NOT USE spinlocks)

    printf("Reader thread started\n");

    /// TODO: CHANGE IT TO WHILE LOOP
    for (int i = 0; i < 10; i++)
    {
        printf("Iteration No-%d\n", i);

        // Open file
        FILE *pFile = open_proc_stat();

        // Read every line from file which starts with "cpu"
        while (1)
        {
            char str[224];
            // Read the line and compare if it starts with "cpu"
            if ((fgets(str, 224, pFile) == NULL) | (strncmp(str, "cpu", (size_t)3) != 0))
            {
                // Does not start with "cpu" -> break loop (because it is last "cpu" line)
                // printf("fgets() == NULL or strncmp() != 0");
                break;
            }
            // Starts with "cpu" -> continue
            printf("%s", str);

            /// TODO: Send string to Analyzer
        }
        printf("\n");

        // Close file and sleep for some time
        fclose(pFile);
        sleep(1);
    }

    printf("Reader thread closes\n");
    return NULL;
}

void *open_proc_stat()
{
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
