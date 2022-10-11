
void *thread_Reader();

// Open /proc/stat file
void *open_proc_stat();

// Read /proc/stat file
void *read_proc_stat(FILE *file);
