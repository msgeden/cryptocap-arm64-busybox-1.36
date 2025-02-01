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

// Wrapper for the custom cap_read syscall.
ssize_t cap_read(int fd, void *buf, size_t count) {
    return read(fd, buf, count);
}


#define PATTERN_LEN 8

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <char-pattern> <total_size> [input_file]\n", argv[0]);
        return EXIT_FAILURE;
    }

    total_size = atol(argv[2])*MB_size;
    if (total_size <= 0) {
        fprintf(stderr, "Invalid size provided.\n");
        return EXIT_FAILURE;
    }
             
    const char *pattern = argv[1];
    size_t pattern_size = strlen(pattern);


    // Open the input file if provided; otherwise, use standard input.
    int fd = STDIN_FILENO;
    if (argc >= 4) {
        fd = open(argv[3], O_RDONLY);
        if (fd < 0) {
            perror("open");
            return EXIT_FAILURE;
        }
    }

    start_t = clock();
    cc_dcap recv_cap_str;
    read(fd, &recv_cap_str, sizeof(cc_dcap));
    printf("Receiver: "); cc_print_cap(recv_cap_str);
    // Allocate a contiguous buffer for the entire data.
    total_size=recv_cap_str.size;
    char *data = malloc(total_size+1);


    if (!data) {
        perror("malloc");
        if (fd != STDIN_FILENO)
            close(fd);
        return EXIT_FAILURE;
    }


    cc_memcpy_i8(data, recv_cap_str, total_size);
    data[total_size] = '\0';
    printf("Receiver: data:%s ", data);

    if (fd != STDIN_FILENO)
        close(fd);

    // Now that the complete data is in memory, search for the pattern.
    unsigned long long occurrences = 0;
    for (size_t i = 0; i <= total_size - pattern_size; i++) {
        if (memcmp(data + i, pattern, pattern_size) == 0)
            occurrences++;
    }
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    printf("\nTotal time (s) of data write (%d MB):\t %f\n", (total_size/1024/1024),total_t);
    
    printf("Found %llu occurrences of pattern \"%s\" in %llu bytes of data.\n",
           occurrences, pattern, total_size);

    free(data);
    return EXIT_SUCCESS;


}
