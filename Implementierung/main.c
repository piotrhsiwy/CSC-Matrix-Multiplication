#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int algorithm_to_use = 0;

struct option options_struct[] = {
        {"a", required_argument, 0, 'a'},
        {"b", required_argument, 0, 'b'},
        {"o", required_argument, 0, 'o'},
        {"V", required_argument, 0, 'v'},
        {"B", optional_argument, 0, 'b'}
};

struct config {
    int algorithm;
    char *matrixA_path;
    char *matrixB_path;
    char *matrix_output_path;
};


int main(int argc, char **argv) {
    // als Default werd, falls er nicht explizit vom Nutzer gesetzt wird
    struct config program_config = {.algorithm = 0};
    int option_index = 0;
    while (true) {
        // shortopts ist noch nicht korrekt, da mal die manpage von getopt(nicht long) anschauen
        // TODO --help
        int ret_val = getopt_long(argc, argv, "a:b:o", options_struct, &option_index);
        if (-1 == ret_val) {
            // alle optionen wurden abgearbeitet
            break;
        }
        // welche option war es denn
        switch (ret_val) {
            size_t length_argument;
            case 'a':
                length_argument = strlen(optarg);
                char *matrixA_path_buffer = malloc(length_argument);
                // TODO ist da eine string funktion besser?
                memcpy(matrixA_path_buffer, optarg, length_argument);
                program_config.matrixA_path = matrixA_path_buffer;
                break;
            case 'b':
                length_argument = strlen(optarg);
                char *matrixB_path_buffer = malloc(length_argument);
                memcpy(matrixB_path_buffer, optarg, length_argument);
                program_config.matrixB_path = matrixB_path_buffer;
                break;
            case 'o':
                length_argument = strlen(optarg);
                char *matrix_output_path_buffer = malloc(length_argument);
                memcpy(matrix_output_path_buffer, optarg, length_argument);
                program_config.matrix_output_path = matrix_output_path_buffer;
                break;
                // TODO die restlichen Argumente
            case 'v':
                // parst die übergebene Zahl von String zu int
                // TODO ist es sicher, NULL als endpointer zu übergeben?
                // Ließt strtol blind weiter, ob schaut es ob der string endet
                program_config.algorithm = strtol(optarg, NULL, 10);
                break;
                // TODO restliche + default mit Error handling
        }
    }
  return 10;
}
