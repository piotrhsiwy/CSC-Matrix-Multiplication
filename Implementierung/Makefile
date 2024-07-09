# TODO handle neither compiler being available
CC := $(shell command -v gcc || command -v clang)

CFLAGS := -std=c17 -Wall -Wextra -Wpedantic

#TODO this is a mess
OBJS := main.o config.o matrix.o benchmark.o mul_algorithms.o mul_scalar.o matrix_util.o dynamic_array.o

all: release
	@echo ------------------------------ Program compiled successfully

debug: CFLAGS += -g -Og
debug: main

release: CFLAGS += -O3
release: main

release_with_symbols: CFLAGS += -O3 -g
release_with_symbols: main

debug_asan: CFLAGS += -g -Og -fsanitize=address
debug_asan: main

debug_ubsan: CFLAGS += -g -Og -fsanitize=undefined
debug_ubsan: main

%.o: **/%.c **/%.h
	@echo "------------------------------ Compiling: " $@
	$(CC) $(CFLAGS) -c -o $@ $<

main: $(OBJS)
	@echo "------------------------------ Linking: " $^
	$(CC) $(CFLAGS) -o main $(OBJS)
	@echo "------------------------------ Cleaning up object files: " $^
	rm *.o

.PHONY: testABC
testABC:
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C.txt

.PHONY: benchABC
benchABC:
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C.txt -B 99999

.PHONY: clean
clean:
	rm main *.o
