#include "signal_io.h"

#include <string.h>
#include <unistd.h>


void print_sig(const char *const msg) {
    __attribute__((unused)) ssize_t ret = write(STDOUT_FILENO, msg, strlen(msg));
}
char getchar_sig(void) {
    char res[2]; //char + \n
    __attribute__((unused)) ssize_t ret = read(STDIN_FILENO, &res, sizeof res / sizeof *res); //TODO: fix having more than 1 char available
    return res[0];
}
