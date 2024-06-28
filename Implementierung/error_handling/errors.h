#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    OK = 0,

    OUT_OF_MEMORY = -1,
    UNEXPECTED_END_OF_FILE = 1,
    UNEXPECTED_END_OF_LINE = 2,
    INVALID_VALUE = 3,
} IO_error;

#define RETURN_IF_NOT_OK(value, return_value)  if (OK != value) return return_value

#endif // ERRORS_H