# TODO handle neither compiler being available
CC := $(shell command -v gcc || command -v clang)

all: main

main: main.c
	$(CC) -o main main.c
clean:
	rm main
