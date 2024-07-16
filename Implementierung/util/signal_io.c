#include "signal_io.h"

#include <string.h>
#include <unistd.h>


void print_sig(const char *const msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}
char getchar_sig(void) {
    char res;
    read(STDIN_FILENO, &res, sizeof res + 1); //char + \n
    return res;
}
