# By default use gcc, if CC arg was not provided (use `make CC=<compiler>`)
CC = gcc

# CFLAGS arg can be used (use `make CFLAGS=<flag>`)
# gcc by default uses gnu17 dialect
# clang by default is ?

ifeq ($(CC),gcc)
# Using `Wall` and `Wextra` flags to get all warnings
	CFLAGS += -Wall -Wextra
else ifeq ($(CC),clang)
# Using `Weverything` flag to get all warnings
	CFLAGS += -Weverything
endif

# ======= functions =======
main: CUT
	@echo "Buid successful"

test: testbuild
	@echo "Test build successful"

run:
	@echo "Executing..."
	@./CUT

testrun:
	@echo "Executing test..."
	@./test

valg: 
	@echo "Starting valgrind inspection..."
	valgrind -v --leak-check=full --show-leak-kinds=all ./CUT

clean:
	@echo "Cleaning..."
	@rm -f *.o CUT test
# =========================

# === build executables ===
CUT: CUT.o reader.o utils.o
	@echo "Making executable..."
	$(CC) $(CFLAGS) -o CUT CUT.o reader.o utils.o

testbuild: test.o utils.o
	@echo "Making executable..."
	$(CC) $(CFLAGS) -o test test.o utils.o
# =========================

# === build *.o files ===
CUT.o: src/CUT.c
	$(CC) $(CFLAGS) -c src/CUT.c

test.o: src/test.c
	$(CC) $(CFLAGS) -c src/test.c

reader.o: src/reader.c src/reader.h
	$(CC) $(CFLAGS) -c src/reader.c

utils.o: src/utils.c src/utils.h
	$(CC) $(CFLAGS) -c src/utils.c
# =========================
