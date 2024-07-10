# TODO handle neither compiler being available
CC := $(shell command -v gcc || command -v clang)

CFLAGS := -std=c17 -Wall -Wextra -Wpedantic

#TODO this is a mess
OBJS := main.o config.o matrix.o benchmark.o mul_algorithms.o mul_scalar.o mul_scalar_expanded.o mul_scalar_expanded_preextract.o matrix_util.o dynamic_array.o

all: release
	@echo ------------------------------ Program compiled successfully

debug: CFLAGS += -g -O0
debug: main

release: CFLAGS += -O3
release: main

release_with_symbols: CFLAGS += -O3 -g
release_with_symbols: main

debug_asan: CFLAGS += -g -O0 -fsanitize=address
debug_asan: main

debug_ubsan: CFLAGS += -g -O0 -fsanitize=undefined
debug_ubsan: main

%.o: **/%.c **/%.h
	@echo "------------------------------ Compiling: " $@
	$(CC) $(CFLAGS) -c -o $@ $<

main: $(OBJS)
	@echo "------------------------------ Linking: " $^
	$(CC) $(CFLAGS) -o main $(OBJS)
	@echo "------------------------------ Cleaning up object files: " $^
	rm *.o

.PHONY: tabc
tabc:
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C0.txt -V 0
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C1.txt -V 1
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C2.txt -V 2
	@echo "----------------------------------------"
	diff matrices/C0.txt matrices/C1.txt
	@echo "----------------------------------------"
	diff matrices/C0.txt matrices/C2.txt
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C0.txt -V 0 -B 4000000
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C1.txt -V 1 -B 4000000
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C1.txt -V 2 -B 4000000
	rm matrices/C0.txt matrices/C1.txt matrices/C2.txt

.PHONY: vg0
vg0:
	valgrind --leak-check=full ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C.txt -V 0
.PHONY: vg1
vg1:
	valgrind --leak-check=full ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C.txt -V 1
.PHONY: vg2
vg2:
	valgrind --leak-check=full ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C.txt -V 2

.PHONY: benchABC
benchABC:
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C.txt -B 99999

.PHONY: clean
clean:
	rm main *.o
