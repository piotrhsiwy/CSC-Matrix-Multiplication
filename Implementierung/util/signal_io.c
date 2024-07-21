#include "signal_io.h"

#include <string.h>
#include <unistd.h>

void print_sig(const char* const msg)
{
    __attribute__((unused)) ssize_t ret = write(STDOUT_FILENO, msg, strlen(msg));
}
char getchar_sig(void)
{
    char res[2]; // char + \n
    __attribute__((unused)) ssize_t ret = read(STDIN_FILENO, &res, sizeof res / sizeof *res);
    if (res[1] != '\n') {
        while (res[1] != '\n' && read(STDIN_FILENO, &res + 1, sizeof *res))
            ; // read any following characters
        return '\0'; // Did not only get one character --> return invalid one
    }
    return res[0];
}
