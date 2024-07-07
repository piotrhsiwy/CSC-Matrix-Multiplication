#ifndef GRA24CAPSPROJEKT_T064_CONFIG_H
#define GRA24CAPSPROJEKT_T064_CONFIG_H

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

#endif //GRA24CAPSPROJEKT_T064_CONFIG_H
