#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    OK = 0,

    OUT_OF_MEMORY = -1,

    UNEXPECTED_END_OF_FILE = 1,
    UNEXPECTED_END_OF_LINE,
    CANNOT_OPEN_FILE,
    CANNOT_CLOSE_FILE,
    INVALID_VALUE,
    INVALID_LINE_LENGTH,
    NO_U64READER_FUNC,

    DIMENSION_MISMATCH = 91,
    INCONSISTENT_RESULTS = 92,

    COULD_NOT_FETCH_TIME = 101,

    UNDEFINED = 255,

} error;

#define RETURN_IF_NOT_OK(value, return_value) \
    if (OK != (value))                        \
    return (return_value)
#define RETURN_IF_NOT_OK_SIMPLE(value) RETURN_IF_NOT_OK(value, value)

const char* error_to_str(error error);

#endif // ERRORS_H
