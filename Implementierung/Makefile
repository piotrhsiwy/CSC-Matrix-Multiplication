# TODO handle neither compiler being available
CC := $(shell command -v gcc || command -v clang)

all: main

debug: CFLAGS := -g -Og
debug: main

release: CFLAGS := -O3
release: main

release_with_symbols: CFLAGS := -O3 -g
release_with_symbols: main

main: main.c
	$(CC) -g -o main main.c
clean:
	rm main
