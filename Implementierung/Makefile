# TODO handle neither compiler being available
CC := $(shell command -v gcc || command -v clang)

CFLAGS := -std=c17 -Wall -Wextra -Wpedantic

#TODO this is a mess
OBJS := main.o config.o matrix.o benchmark.o mul_algorithms.o mul_scalar.o mul_scalar_expanded.o mul_scalar_expanded_preextract.o mul_vector.o mul_gustavson.o matrix_util.o dynamic_array.o

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

# matrices/C.txt is a reference solution
.PHONY: tabc
tabc:
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C0.txt -V 0
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C1.txt -V 1
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C2.txt -V 2
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C3.txt -V 3
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C4.txt -V 4
	@echo "----------------------------------------"
	diff matrices/C.txt matrices/C0.txt
	@echo "----------------------------------------"
	diff matrices/C.txt matrices/C1.txt
	@echo "----------------------------------------"
	diff matrices/C.txt matrices/C2.txt
	@echo "----------------------------------------"
	diff matrices/C.txt matrices/C3.txt
	@echo "----------------------------------------"
	diff matrices/C.txt matrices/C4.txt
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C0.txt -V 0 -B 4000000
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C1.txt -V 1 -B 4000000
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C2.txt -V 2 -B 4000000
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C3.txt -V 3 -B 4000000
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/C4.txt -V 4 -B 4000000
	rm matrices/C0.txt matrices/C1.txt matrices/C2.txt matrices/C3.txt matrices/C4.txt

.PHONY: vg0
vg0:
	valgrind -s --leak-check=full ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C0.txt -V 0
.PHONY: vg1
vg1:
	valgrind -s --leak-check=full ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C1.txt -V 1
.PHONY: vg2
vg2:
	valgrind -s --leak-check=full ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C2.txt -V 2
.PHONY: vg3
vg3:
	valgrind -s --leak-check=full ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C3.txt -V 3

.PHONY: pf0
pf0:
	perf record ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C0.txt -V 0
	perf report
.PHONY: pf1
pf1:
	perf record ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C1.txt -V 1
	perf report
.PHONY: pf2
pf2:
	perf record ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C2.txt -V 2
	perf report
.PHONY: pf3
pf3:
	perf record ./main -a matrices/A.txt -b matrices/B.txt -o matrices/C3.txt -V 3
	perf report


.PHONY: benchABC
benchABC:
	./main -a matrices/A.txt -b matrices/B.txt -o matrices/Cbench.txt -B 99999

.PHONY: clean
clean:
	rm main *.o -f
