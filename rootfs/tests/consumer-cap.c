/*
 * consumer_known_size.c
 *
 * This program assumes that both the producer and consumer know the total data size in advance.
 * It allocates a buffer of that exact size, reads the entire data using the custom cap_read syscall,
 * and then searches the complete buffer for a specified 8-character pattern.
 *
 * Usage:
 *     ./consumer_known_size <8-char-pattern> <total_size> [input_file]
 *
 * If no input file is provided, the program reads from standard input.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/syscall.h>

#include "cc_header.h"

size_t total_size=0;
clock_t start_t, end_t;
double total_t;

#define PATTERN_LEN 8

int main(int argc, char *argv[]) {

    char *pattern = "*";

    if (argc > 1) 
        pattern=argv[1];

    size_t pattern_size = strlen(pattern);


    pid_t producer_pid=0;
    read(STDIN_FILENO, &producer_pid, sizeof(pid_t));
    cc_dcap recv_cap_str;
    cc_read_cap(STDIN_FILENO, &recv_cap_str);

    // Allocate a contiguous buffer for the entire data.
    total_size=recv_cap_str.size;
    start_t = clock();
    char *data = malloc(total_size);
    cc_memcpy_i8_asm(data, recv_cap_str, total_size);
    end_t = clock();
    printf("Receiver: data:%s ", data);
    total_t = (double)(end_t - start_t)/CLOCKS_PER_SEC;
    printf("\nTotal time (s) of single copy (%.2f MB) via cap-based pipe:\t %.4f\n", (double)total_size/MB_size,total_t);

    // Now that the complete data is in memory, search for the pattern.
    unsigned long long occurrences = 0;
    for (size_t i = 0; i <= total_size - pattern_size; i++) {
        if (memcmp(data + i, pattern, pattern_size) == 0)
            occurrences++;
    }
    
    printf("Found %llu occurrences of pattern \"%s\" in %lu MB of data.\n",
           occurrences, pattern, total_size/MB_size);

    cc_resume_process(producer_pid);
    free(data);
    return EXIT_SUCCESS;


}
