# TODO handle neither compiler being available
CC := $(shell command -v gcc || command -v clang)

CFLAGS := -std=c17 -Wall -Wextra -Wpedantic

OBJS := main.o matrix_mult.o matrix.o

all: main
	@echo ------------------------------ Program compiled successfully

debug: CFLAGS += -g -Og
debug: main

release: CFLAGS += -O3
release: main

release_with_symbols: CFLAGS += -O3 -g
release_with_symbols: main

%.o: %.c %.h
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

.PHONY: clean
clean:
	rm main
