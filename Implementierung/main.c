#include "matrix.h"
#include "matrix_mult.h"

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
// TODO make it return IO_ERROR and put the return value in an output parameter
// TODO also IO_ERROR should be renamed to just ERROR
IO_error allocate_argument(const char* const optarg_to_process, char** target)
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
int parse_number(const char* const optarg_to_process)
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
    return (int)value;
}


IO_error parse_arguments(const int argc,const char** const argv, config *program_config)
{
    int option_index = 0;
    while (true) {
        // : nach buchstabe -> required_argument, :: -> optional argument
        // nichts nach buchstaben -> kein argument
        const int ret_val = getopt_long(argc, argv, "a:b:o:V:b::h", options_struct, &option_index);
        // TODO check errno maybe?
        if (-1 == ret_val) {
            // alle optionen wurden abgearbeitet
            break;
        }
        // So muss nicht im Switch die Variable erstellt werden
        // ist glaube ich erst mit c23 erlaubt, aber wir nutzen c17
        size_t length_argument;
        char* temp;
        IO_error  err_return_value;
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
        default:
            // TODO handle all possible return values of getopt_long
            break;
        }
    }
    return OK;
}

int main(int argc, char** argv)
{
    // Replace with file of your liking
    // FILE* f = fopen("/mnt/c/Users/nikpe/Downloads/test.csc", "r");
    // matrix_data m;
    // IO_error err = read_matrix_from_fd(f, &m);

    config options = { .algorithm = 0, .requested_help = false, .is_benchmark = false };
    // TODO write function to deallocate buffers in config
    IO_error err_parse_config = parse_arguments(argc, argv, &options);
    if (OK != err_parse_config) {
        // TODO proper error handling
        return 234;
    }
    if (options.requested_help) {
        // TODO Ernsthafte Help
        const char* const help_str = "Das Programm unterstützt folgende Optionen:\n"
                         "-a <Pfad zur Datei> : Pfad zur Datei, in der die erste Matrix, die mulitpliziert werden soll liegt.\n"
                         "-b <Pfad zur Datei> : Pfad zur Datei, in der die erste Matrix, die multipliziert werden soll liegt.\n"
                         "-o <Pfad zur Datei> : Pfad, an dem das Ergebnis der Multiplikation geschrieben werden sollte.\n\n"
                                     "-V <Zahl>: Wählt den Algorithmus, der zur Multiplikation verwendet werden soll. Folgende sind verfügbar: \n"
                                     "\t0: der normale Algorithmus. TODO was macht er\n"
                                     "-B : misst die Laufzeit der Verschiedenen Algorithmen auf einigen Matrixen. Diese Daten werden dann ausgegeben.\n"
                                     "-B <Zahl>: Wie '-B', aber die Berechnung wird <Zahl> mal durchgeführt.\n\n"
                                     "-h oder --help: Gibt diese Hilfe aus.\n";

        printf("%s", help_str);
        return 0;
    }
    // Did the user pass all required options?
    // TODO ist das garantiert NULL falls nicht intialisiert
    if (NULL == options.matrixA_path) {
        // TODO bessere Fehlermeldung
        printf("Ein Pfad für die Matrix A muss spezifiziert werden.\n");
        // TODO standard unix return codes
        // TODO cleanup
        return 4;
    }
    if (NULL == options.matrixB_path) {
        printf("Ein Pfad für die Matrix B muss spezifiziert werden.\n");
        return 5;
    }
    if (NULL == options.matrix_output_path) {
        printf("Ein Pfad an dem die Ergebnis-Matrix geschrieben werden soll muss spezifiziert werden.\n");
        return 6;
    }

    // read the matrixes from the specified paths
    FILE* matrix_a_file = fopen(options.matrixA_path, "r");
    FILE* matrix_b_file = fopen(options.matrixB_path, "r");
  
    matrix_data a;
    matrix_data b;
    IO_error err_a = read_matrix_from_fd(matrix_a_file, &a);
    IO_error err_b = read_matrix_from_fd(matrix_b_file, &b);
    if (OK != err_a) {
        // TODO proper error handling
        return 1;
    }
    if (OK != err_b) {
        return 2;
    }

    matrix_data output;
    matrix_mult_csc(&a, &b, &output);

    // now write the output
    // TODO evtl printen??
    // TODO handle fopen failing (also for matrixA + matrixB)
    FILE* matrix_output_file = fopen(options.matrix_output_path, "w");
    IO_error err_output = write_matrix_to_fd(matrix_output_file, output);

    destruct_matrix_data(&a);
    destruct_matrix_data(&b);
    destruct_matrix_data(&output);
    fclose(matrix_a_file);
    fclose(matrix_b_file);
    fclose(matrix_output_file);

    if (OK != err_output) {
        // TODO better error handling
        return 3;
    }
    return 10;
}
