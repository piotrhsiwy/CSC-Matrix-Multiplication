#include "config.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct option options_struct[] = {
    { "help", no_argument, 0, 'h' }
};

/*
 * allocates a buffer for the argument and copies it there.
 * Return NULL if allocation of the buffer failed
 */
__attribute__((cold)) error allocate_argument(const char* const optarg_to_process, char** target)
{
    const size_t length = strlen(optarg_to_process);
    // Danke Piotr
    char* buffer = malloc(length + 1);
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
__attribute__((cold)) long parse_number(const char* const optarg_to_process) {
    errno = 0;
    const long value = strtol(optarg_to_process, NULL, 10);
    if (errno) {
        fprintf(stderr, "Ungültige Zahl: %s (errno von strtol: %d)\n", optarg_to_process, errno);
        errno = 0;
    }
    // TODO do we actually care?
    // we dont care about negative numbers or numbers above INT32_MAX
    return value;
}

__attribute__((cold)) error parse_arguments(const int argc, char* const* const argv, config* const program_config)
{
    int option_index = 0;
    while (true) {
        // : nach buchstabe -> required_argument, :: -> optional argument
        // nichts nach buchstaben -> kein argument
        const int ret_val = getopt_long(argc, argv, ":a:b:o:V:B:U:h", options_struct, &option_index);
        // TODO check errno maybe?
        if (-1 == ret_val) {
            // alle optionen wurden abgearbeitet
            break;
        }
        // So muss nicht im Switch die Variable erstellt werden
        // ist glaube ich erst mit c23 erlaubt, aber wir nutzen c17
        error err_return_value;
        // zeigt was gerade geparst wurde
        switch (ret_val) {
        case 'a':
            err_return_value = allocate_argument(optarg, &program_config->matrixA_path);
            RETURN_IF_NOT_OK_SIMPLE(err_return_value);
            break;
        case 'b':
            err_return_value = allocate_argument(optarg, &program_config->matrixB_path);
            RETURN_IF_NOT_OK_SIMPLE(err_return_value);
            break;
        case 'o':
            err_return_value = allocate_argument(optarg, &program_config->matrix_output_path);
            RETURN_IF_NOT_OK_SIMPLE(err_return_value);
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
        case 'U':
            program_config->is_unattended_benchmark = true;
            err_return_value = allocate_argument(optarg, &program_config->unattended_export_file);
            RETURN_IF_NOT_OK_SIMPLE(err_return_value);
            break;
        case 'h':
            // TODO soll checken if requested_help und benchmark gleichzeitig
            program_config->requested_help = true;
            break;
        case ':':
            // is it actually B or did the user forget the argument for something else
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

void print_config_struct(__attribute__((unused))config option) {
    // printf("");
}

// to run this unit-test, put it somewhere in the main. Most of the time, the user doesnt want to unit-test so its not included.
void unit_test_parse_arguments(void) {
    // do the basics work?
    const char *input1[9];
    input1[0] = "./main";
    input1[1] = "-a";
    input1[2] = "matrices/A.txt";
    input1[3] = "-b";
    input1[4] = "matrices/B.txt";
    input1[5] = "-o";
    input1[6] = "matrices/C.txt";
    input1[7] = "-V";
    input1[8] = "1";
    config options1 = {.benchmark_repeat_count = 10000, .algorithm = 0, .requested_help = false, .is_benchmark = false, .is_unattended_benchmark = false, .unattended_export_file = NULL, .matrix_output_path = NULL, .matrixB_path = NULL, .matrixA_path = NULL};
    error output1 = parse_arguments(7, (char* const*)input1, &options1);
    if (output1 != OK) {
        fprintf(stderr, "Testcase 1 should work fine, but it returned an error.\n");
    }
    if (strcmp(options1.matrixA_path, "matrices/A.txt") != 0) {
        fprintf(stderr, "Testcase 1: matrixA_path should be 'matrices/A.txt' but its '%s' instead.\n", options1.matrixA_path);
    }
    if (strcmp(options1.matrixB_path, "matrices/B.txt") != 0) {
        fprintf(stderr, "Testcase 1: matrixB_path should be 'matrices/B.txt' but its '%s' instead.\n", options1.matrixB_path);
    }
    if (strcmp(options1.matrix_output_path, "matrices/C.txt") != 0) {
        fprintf(stderr, "Testcase 1: matrix_output_path should be 'matrices/C.txt' but its '%s' instead.\n", options1.matrix_output_path);
    }
    if (options1.algorithm != 1) {
        fprintf(stderr, "Testcase 1: algorithm should be '1' but is '%ld'.\n", options1.algorithm);
    }
    // Cleanup Testcase 1
    free(options1.matrix_output_path);
    free(options1.matrixA_path);
    free(options1.matrixB_path);

    // does help work?
    const char* input2[2];
    input2[0] = "./main";
    input2[1] = "--help";
    config options2 = {.benchmark_repeat_count = 10000, .algorithm = 0, .requested_help = false, .is_benchmark = false, .is_unattended_benchmark = false, .unattended_export_file = NULL, .matrix_output_path = NULL, .matrixB_path = NULL, .matrixA_path = NULL};
    error output2 = parse_arguments(1, (char* const*)input2, &options2);
    if (output2 != OK) {
        fprintf(stderr, "Testcase 2: should be sucessful, but another returnvalue was returned.\n");
    }
    if (options2.requested_help != true) {
        fprintf(stderr, "Testcase 2: --help passed, but request_help is false.\n");
    }
    // TODO more
    printf("Finished with the unit-test!\n");
}
