# CPU Usage Tracker

#### Description:
This program reads cpu usage lines from `/proc/stat` file, calculates percentage and then prints it to console.

It uses c `pthreads` library and has 3 threads that have corresponding tasks:
- Reader: reads `/proc/stat`, finds every `cpu` line (which starts with `cpu_`), extracts `cpu fields` into the structure and sends these structs as a group into Analyzer buffer.
- Analyzer: receives group from Reader buffer, calculates usage percentages for each cpu and sends them as a group to Printer buffer.
- Printer: every 1 second takes all groups from Analyzer buffer, averages percentages for each cpu and prints it to console.

Currently not implemented: 
- Watchdog: watch for deadlocked threads. (have thoughts how I would implement it)
- Logger: log helpfull debug messages to file.

Between-thread communication solved with "*Consumer-Producer problem*".

Also proper signal handling implemented, in order for program to exit properly.

#### Build system:
It uses make and Makefile.

Commands: 
- `make`: build executable. By default uses `gcc` compiler, but can be compiled with `clang` with flag `CC=clang`. Additionally you can provide `CFLAGS` for compiler.
- `make run`: run executable.
- `make valg`: run with Valgrind to check for memory leaks.
- `make clean`: clean `.o` files.
