#include "signal_io.h"

#include <string.h>
#include <unistd.h>


void print_sig(const char *const msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}
char getchar_sig(void) {
    char res[2]; //char + \n
    read(STDIN_FILENO, &res, sizeof res / sizeof *res); //TODO: fix having more than 1 char available
    return res[0];
}
