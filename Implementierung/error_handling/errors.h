#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    OK = 0,

    OUT_OF_MEMORY = -1,

    UNEXPECTED_END_OF_FILE = 1,
    UNEXPECTED_END_OF_LINE = 2,
    CANNOT_OPEN_FILE = 3,
    INVALID_VALUE = 4,
    DIMENSION_MISMATCH = 5,

    COULD_NOT_FETCH_TIME = 101,

} error;

#define RETURN_IF_NOT_OK(value, return_value)  if (OK != value) return return_value

#endif // ERRORS_H
