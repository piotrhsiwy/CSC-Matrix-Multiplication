#include "matrix.h"
#include "config.h"
#include "algorithms/mul_algorithms.h"
#include "benchmark.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void deallocate_config_buffers(config *options) {
    // freeing NULL doesnt do anything, no need to check for it
    free(options->matrix_output_path);
    options->matrix_output_path = NULL;
    free(options->matrixA_path);
    options->matrixA_path = NULL;
    free(options->matrixB_path);
    options->matrixB_path = NULL;
}


int main(const int argc, char** argv)
{
    //TODO: switch to start unattended benchmark and configure iterations per run

    config options = { .algorithm = 0, .requested_help = false, .is_benchmark = false };
    // TODO write function to deallocate buffers in config
    error err_parse_config = parse_arguments(argc, argv, &options);
    if (OK != err_parse_config) {
        // TODO proper error handling
        deallocate_config_buffers(&options);
        return EXIT_FAILURE;
    }
    if (options.requested_help) {
        // TODO Ernsthafte Help
        const char* const help_str = "Das Programm unterstützt folgende Optionen:\n"
                         "-a <Pfad zur Datei> : Pfad zur Datei, in der die erste Matrix, die mulitpliziert werden soll liegt.\n"
                         "-b <Pfad zur Datei> : Pfad zur Datei, in der die zweite Matrix, die multipliziert werden soll liegt.\n"
                         "-o <Pfad zur Datei> : Pfad, an dem das Ergebnis der Multiplikation geschrieben werden sollte.\n\n"
                                     "-V <Zahl>: Wählt den Algorithmus, der zur Multiplikation verwendet werden soll. Folgende sind verfügbar: \n"
                                     "\t0: der normale Algorithmus. TODO was macht er\n"
                                     "-B : misst die Laufzeit der Verschiedenen Algorithmen auf einigen Matrixen. Diese Daten werden dann ausgegeben.\n"
                                     "-B <Zahl>: Wie '-B', aber die Berechnung wird <Zahl> mal durchgeführt.\n\n"
                                     "-h oder --help: Gibt diese Hilfe aus.\n";

        printf("%s", help_str);
        deallocate_config_buffers(&options);
        return EXIT_SUCCESS;
    }
    // Did the user pass all required options?
    // TODO ist das garantiert NULL falls nicht intialisiert
    if (NULL == options.matrixA_path) {
        // TODO bessere Fehlermeldung
        printf("Ein Pfad für die Matrix A muss spezifiziert werden.\n");
        // TODO cleanup
        deallocate_config_buffers(&options);
        return EXIT_FAILURE;
    }
    if (NULL == options.matrixB_path) {
        printf("Ein Pfad für die Matrix B muss spezifiziert werden.\n");
        deallocate_config_buffers(&options);
        return EXIT_FAILURE;
    }
    if (NULL == options.matrix_output_path) {
        printf("Ein Pfad an dem die Ergebnis-Matrix geschrieben werden soll muss spezifiziert werden.\n");
        deallocate_config_buffers(&options);
        return EXIT_FAILURE;
    }

    // read the matrixes from the specified paths
    FILE* matrix_a_file = fopen(options.matrixA_path, "r");
    if(!matrix_a_file){
        fprintf(stderr, "Cannot open file %s\n", options.matrixA_path);
        return CANNOT_OPEN_FILE;
    }
    FILE* matrix_b_file = fopen(options.matrixB_path, "r");
    if(!matrix_b_file){
        fprintf(stderr, "Cannot open file %s\n", options.matrixB_path);
        return CANNOT_OPEN_FILE;
    }
  
    matrix_data a;
    matrix_data b;
    error err_a = read_matrix_from_fd(matrix_a_file, &a);
    error err_b = read_matrix_from_fd(matrix_b_file, &b);
    if (OK != err_a) {
        // TODO proper error handling
        deallocate_config_buffers(&options);
        return EXIT_FAILURE;
    }
    if (OK != err_b) {
        deallocate_config_buffers(&options);
        return EXIT_FAILURE;
    }

    matrix_data output;
    matrix_multiplier algorithm = id_to_algorithm(options.algorithm);
    if(!algorithm) {
        fprintf(stderr, "Unbekannter Algorithmus. Siehe -h bzw. --help für mehr Informationen.\n");
        deallocate_config_buffers(&options);
        return EXIT_FAILURE;
    }

    if(options.is_benchmark) {
        struct timespec duration;
        output.mul_error = execute_benchmark(&a, &b, &output, options.benchmark_repeat_count, algorithm, &duration);

        if (output.mul_error == OK) {
            printf("%ld multiplications took %ld seconds and %ld nano seconds.\n",
                   options.benchmark_repeat_count, duration.tv_sec, duration.tv_nsec);
        }
    } else {
        algorithm(&a, &b, &output);
    }

    //TODO: handle mul_error

    // now write the output
    // TODO evtl printen??
    // TODO handle fopen failing (also for matrixA + matrixB)
    FILE* matrix_output_file = fopen(options.matrix_output_path, "w");
    if(!matrix_output_file){
        fprintf(stderr, "Cannot open file %s\n", options.matrix_output_path);
        return CANNOT_OPEN_FILE;
    }
    error err_output = write_matrix_to_fd(matrix_output_file, output);

    if(fclose(matrix_a_file)){
        fprintf(stderr, "Cannot close file %s\n", options.matrixA_path);
        return CANNOT_CLOSE_FILE;   
    }
    if(fclose(matrix_b_file)){
        fprintf(stderr, "Cannot close file %s\n", options.matrixB_path);
        return CANNOT_CLOSE_FILE;   
    }
    if(fclose(matrix_output_file)){
        fprintf(stderr, "Cannot close file %s\n", options.matrix_output_path);
        return CANNOT_CLOSE_FILE;   
    }

    // funnily enough, the data is written to the files with fflush() only after the files are close, so I have to wait with freeing any memory until then
    deallocate_config_buffers(&options);
    destruct_matrix_data(&a);
    destruct_matrix_data(&b);
    if(output.mul_error == OK){
        destruct_matrix_data(&output);
    }

    if (OK != err_output) {
        // TODO better error handling
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
