# By default use gcc, if CC option was not provided (`make CC=<compiler>`)
CC = gcc

# Using `std=c99` flag to compile with c99 standart
CFLAGS = -std=c99

ifeq ($(CC),gcc)
# Using `Wall` and `Wextra` flags to get all warnings
	CFLAGS += -Wall -Wextra
else ifeq ($(CC),clang)
# Using `Weverything` flag to get all warnings
	CFLAGS += -Weverything
endif

main: CUT.o 
	@echo "Making executable..."
	$(CC) $(CFLAGS) -o CUT CUT.o

CUT.o:
	@echo "Making objects..."
	$(CC) $(CFLAGS) -c src/CUT.c

run:
	@echo "Executing..."
	@./CUT

clean:
	@echo "Cleaning..."
	@rm -f *.o CUT