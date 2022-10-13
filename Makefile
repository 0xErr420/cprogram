# By default use gcc, if CC option was not provided (use `make CC=<compiler>`)
CC = gcc

# By default using `std` flag to compile with c99 standart, 
# if CFLAGS option was not provided (use `make CFLAGS=-std=<version>`)
CFLAGS = -std=c99

ifeq ($(CC),gcc)
# Using `Wall` and `Wextra` flags to get all warnings
	CFLAGS += -Wall -Wextra
else ifeq ($(CC),clang)
# Using `Weverything` flag to get all warnings
	CFLAGS += -Weverything
endif

main: CUT
	@echo "Buid successful"

CUT: CUT.o reader.o utils.o
	@echo "Making executable..."
	$(CC) $(CFLAGS) -o CUT CUT.o reader.o utils.o

CUT.o: src/CUT.c
	$(CC) $(CFLAGS) -c src/CUT.c

reader.o: src/reader.c src/reader.h
	$(CC) $(CFLAGS) -c src/reader.c

utils.o: src/utils.c src/utils.h
	$(CC) $(CFLAGS) -c src/utils.c

run:
	@echo "Executing..."
	@./CUT

clean:
	@echo "Cleaning..."
	@rm -f *.o CUT