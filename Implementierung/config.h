#ifndef GRA24CAPSPROJEKT_T064_CONFIG_H
#define GRA24CAPSPROJEKT_T064_CONFIG_H

#include "error_handling/errors.h"

#include <stdbool.h>


typedef struct {
    long algorithm;
    char* matrixA_path;
    char* matrixB_path;
    char* matrix_output_path;
    bool requested_help;
    bool is_benchmark;
    bool is_unattended_benchmark;
    char *unattended_export_file;
    long benchmark_repeat_count;
} config;

/**
 * Parses the cli arguments passed to the function and updates the values in program_config accordingly
 * @param argc the argc received in the main method
 * @param argv the argv received in the main method
 * @param program_config the struct whose values should be updated
 * @returns Possible Values: OK | OUT_OF_MEMORY
 */
error parse_arguments(int argc, char *const * argv, config* program_config);

#endif //GRA24CAPSPROJEKT_T064_CONFIG_H
