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
CUT: CUT.o reader.o analyzer.o utils.o group.o circular_buffer.o
	@echo "Making executable..."
	$(CC) $(CFLAGS) -o $@ $^

testbuild: test.o utils.o
	@echo "Making executable..."
	$(CC) $(CFLAGS) -o test $^
# =========================

# === build *.o files ===
%.o: src/%.c
	$(CC) $(CFLAGS) -c $<

CUT.o: src/CUT.c
test.o: src/test.c

reader.o: src/reader.c src/reader.h
analyzer.o: src/analyzer.c src/analyzer.h
utils.o: src/utils.c src/utils.h
group.o: src/group.c src/group.h
circular_buffer.o: src/circular_buffer.c src/circular_buffer.h
# =========================
