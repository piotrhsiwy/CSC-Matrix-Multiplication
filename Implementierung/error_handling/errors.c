#include "errors.h"


const char* error_to_str(const error err) {
    switch (err) {
    case OK:
        return "OK";
    case OUT_OF_MEMORY:
        return "Out of memory";
    case UNEXPECTED_END_OF_FILE:
        return "Unexpected end of file";
    case UNEXPECTED_END_OF_LINE:
        return "Unexpected end of line";
    case CANNOT_OPEN_FILE:
        return "Cannot open file";
    case CANNOT_CLOSE_FILE:
        return "Cannot close file";
    case INVALID_VALUE:
        return "Invalid value";
    case DIMENSION_MISMATCH:
        return "Dimension mismatch";
    case COULD_NOT_FETCH_TIME:
        return "Could not fetch time";
    case UNDEFINED:
        return "Undefined";
    }
}
