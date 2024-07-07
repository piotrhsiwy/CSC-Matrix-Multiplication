#ifndef GRA24CAPSPROJEKT_T064_CONFIG_H
#define GRA24CAPSPROJEKT_T064_CONFIG_H

#include "error_handling/errors.h"

#include <stdbool.h>


typedef struct {
    int algorithm;
    char* matrixA_path;
    char* matrixB_path;
    char* matrix_output_path;
    bool requested_help;
    bool is_benchmark;
    unsigned int benchmark_repeat_count;
} config;

IO_error parse_arguments(int argc,const char** argv, config* program_config);

#endif //GRA24CAPSPROJEKT_T064_CONFIG_H
