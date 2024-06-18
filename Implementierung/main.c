#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct option options_struct[] = {
    // TODO ich glaube davon ist nur help notwendig, der rest sind ein Buchstabe optionen
    { "a", required_argument, 0, 'a' },
    { "b", required_argument, 0, 'b' },
    { "o", required_argument, 0, 'o' },
    { "V", required_argument, 0, 'v' },
    { "B", optional_argument, 0, 'b' },
    { "help", no_argument, 0, 'h' }
};

typedef struct {
    int algorithm;
    char* matrixA_path;
    char* matrixB_path;
    char* matrix_output_path;
    bool requested_help;
    bool is_benchmark;
    int benchmark_repeat_count;
} config;

/*
 * allocates a buffer for the argument and copies it there.
 * Return NULL if allocation of the buffer failed
 */
char* allocate_argument(char* optarg_to_process)
{
    size_t length = strlen(optarg_to_process);
    char* buffer = malloc(length);
    // TODO mehr error handling?
    if (NULL == buffer) {
        return NULL;
    }
    memcpy(buffer, optarg_to_process, length);
    return buffer;
}

int parse_number(char* optarg_to_process)
{
    long value = strtol(optarg_to_process, NULL, 10);
    errno = 0;
    if (errno) {
        // TODO handle error;
    }
    // TODO do we actually care?
    // we dont care about negative numbers or numbers above INT32_MAX
    return (int)value;
}

config parse_arguments(int argc, char** argv)
{
    // default values
    config program_config = { .algorithm = 0, .requested_help = false, .is_benchmark = false };
    int option_index = 0;
    while (true) {
        // : nach buchstabe -> required_argument, :: -> optional argument
        // nichts nach buchstaben -> kein argument
        int ret_val = getopt_long(argc, argv, "a:b:o:V:b::h", options_struct, &option_index);
        // TODO check errno maybe?
        if (-1 == ret_val) {
            // alle optionen wurden abgearbeitet
            break;
        }
        // So muss nicht im Switch die Variable erstellt werden
        // ist glaube ich erst mit c23 erlaubt, aber wir nutzen c17
        size_t length_argument;
        char* temp;
        // zeigt was gerade geparst wurde
        switch (ret_val) {
        case 'a':
            temp = allocate_argument(optarg);
            // TODO how to handle this?
            // Just abort??
            if (NULL == temp) {
                break;
            }
            program_config.matrixA_path = temp;
            break;
        case 'b':
            temp = allocate_argument(optarg);
            // TODO how to handle this?
            // Just abort?
            if (NULL == temp) {
                break;
            }
            program_config.matrixB_path = temp;
            break;
        case 'o':
            temp = allocate_argument(optarg);
            // TODO how to handle this?
            // Just abort
            if (NULL == temp) {
                break;
            }
            program_config.matrix_output_path = temp;
            break;
        case 'V':
            program_config.algorithm = parse_number(optarg);
            break;
        case 'B':
            program_config.is_benchmark = true;
            program_config.benchmark_repeat_count = parse_number(optarg);
            // TODO how do optional arguments work??
            // i dont think this works if -B used without argument
            break;
        case 'h':
            // TODO soll checken if requested_help und benchmark gleichzeitig
            program_config.requested_help = true;
            break;
        default:
            // TODO handle all possible return values of getopt_long
            break;
        }
    }
    return program_config;
}

int main(int argc, char** argv)
{
    config options = parse_arguments(argc, argv);
    if (options.requested_help) {
        // TODO Ernsthafte Help
        printf("hier sollte eine Hilfe sein.\n");
    }
    return 10;
}
