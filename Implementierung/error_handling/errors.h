#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    OK = 0,

    OUT_OF_MEMORY = -1,

    UNEXPECTED_END_OF_FILE,
    UNEXPECTED_END_OF_LINE,
    CANNOT_OPEN_FILE,
    CANNOT_CLOSE_FILE,
    INVALID_VALUE,
    DIMENSION_MISMATCH,

    COULD_NOT_FETCH_TIME = 101,

} error;

#define RETURN_IF_NOT_OK(value, return_value)  if (OK != value) return return_value

#endif // ERRORS_H
