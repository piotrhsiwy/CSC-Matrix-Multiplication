#include "config.h"

#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>


struct option options_struct[] = {
        // TODO ich glaube davon ist nur help notwendig, der rest sind ein Buchstabe optionen
        { "help", no_argument, 0, 'h' }
};

/*
 * allocates a buffer for the argument and copies it there.
 * Return NULL if allocation of the buffer failed
 */
// TODO make it return IO_ERROR and put the return value in an output parameter
// TODO also IO_ERROR should be renamed to just ERROR
__attribute__((cold)) error allocate_argument(const char* const optarg_to_process, char** target)
{
    const size_t length = strlen(optarg_to_process);
    // Danke Piotr
    char* buffer = malloc(length + 1);
    // TODO mehr error handling?
    if (NULL == buffer) {
        return OUT_OF_MEMORY;
    }
    // TODO use a string function instead
    memcpy(buffer, optarg_to_process, length);
    buffer[length] = '\0';
    *target = buffer;
    return OK;
}
// TODO refactor to use long everywhere
__attribute__((cold)) long parse_number(const char* const optarg_to_process)
{
    const long value = strtol(optarg_to_process, NULL, 10);
    errno = 0;
    if (errno) {
        // TODO handle error;
        // Hier müsste der Error geprintet werden
        // vgl errno_funktionen
    }
    // TODO do we actually care?
    // we dont care about negative numbers or numbers above INT32_MAX
    return value;
}


__attribute__((cold)) error parse_arguments(const int argc, char *const * const argv, config *const program_config)
{
    int option_index = 0;
    while (true) {
        // : nach buchstabe -> required_argument, :: -> optional argument
        // nichts nach buchstaben -> kein argument
        const int ret_val = getopt_long(argc, argv, ":a:b:o:V:B:h", options_struct, &option_index);
        // TODO check errno maybe?
        if (-1 == ret_val) {
            // alle optionen wurden abgearbeitet
            break;
        }
        // So muss nicht im Switch die Variable erstellt werden
        // ist glaube ich erst mit c23 erlaubt, aber wir nutzen c17
        size_t length_argument;
        char* temp;
        error  err_return_value;
        // zeigt was gerade geparst wurde
        switch (ret_val) {
            case 'a':
                err_return_value = allocate_argument(optarg, &program_config->matrixA_path);
                // TODO how to handle this?
                // are there other possible errors
                // Just abort??
                RETURN_IF_NOT_OK (err_return_value, OUT_OF_MEMORY);
                break;
            case 'b':
                err_return_value = allocate_argument(optarg, &program_config->matrixB_path);
                // TODO how to handle this?
                // Just abort?
                RETURN_IF_NOT_OK(err_return_value, OUT_OF_MEMORY);
                break;
            case 'o':
                err_return_value = allocate_argument(optarg, &program_config->matrix_output_path);
                // TODO how to handle this?
                // Just abort
                RETURN_IF_NOT_OK(err_return_value, OUT_OF_MEMORY);
                break;
            case 'V':
                // TODO handle invalid arguments (i.e. things that arent a number)
                program_config->algorithm = parse_number(optarg);
                break;
            case 'B':
                program_config->is_benchmark = true;
                program_config->benchmark_repeat_count = parse_number(optarg);
                // TODO how do optional arguments work??
                // i dont think this works if -B used without argument
                break;
            case 'h':
                // TODO soll checken if requested_help und benchmark gleichzeitig
                program_config->requested_help = true;
                break;
            case ':':
                program_config->is_benchmark = true;
                program_config->benchmark_repeat_count = 1;
                break;
            default:
                // TODO handle all possible return values of getopt_long
                break;
        }
    }
    return OK;
}
