#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    ok = 0,

    out_of_memory = -1,
    unexpected_end_of_file = 1,
    unexpected_end_of_line = 2,
    invalid_value = 3,
} IO_error;

#endif // ERRORS_H