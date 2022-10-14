#include <stdio.h>

/// String size of 'cpu' line
///
/// Should be enough, assuming that in the file /proc/stat the line 'cpu' has a maximum of 10 fields
/// calculated: <MAX_unsigned_long_long> * <max number of fileds in 'cpu' line> + <sizeof("cpu ")>
#define CPU_READ_SIZE 224UL

void *thread_Reader();

// Open /proc/stat file
void *open_proc_stat();

// Read /proc/stat file
void *read_proc_stat(FILE *file);
