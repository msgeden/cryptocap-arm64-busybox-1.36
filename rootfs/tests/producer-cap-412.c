/*
 * producer_ready.c
 *
 * This program generates a large contiguous block of random text in memory and then writes it
 * using your custom cap_write syscall in one call. This ensures that the entire data is ready
 * before the write operation is initiated.
 *
 * Compile with:
 *     gcc -o producer_ready producer_ready.c
 *
 * Usage:
 *     ./producer_ready [total_bytes]
 *
 * Note: For huge total_bytes, ensure that you have sufficient memory available.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>

#include "cc_header.h"

size_t total_size=0;
clock_t start_t, end_t;
double total_t;

int main(int argc, char *argv[]) {
    // Default total size: 100 MB.
    if (argc > 1) {
        total_size = atol(argv[1])*MB_size;
        if (total_size == 0) {
            fprintf(stderr, "Invalid size provided.\n");
            return EXIT_FAILURE;
        }
    }
    
    // Allocate a single contiguous block of memory for the entire data.
    char *buffer = malloc(total_size+1);
    //printf("Sender: malloc address:0x%lx\n", buffer);

    if (!buffer) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    
    // Seed the random number generator.
    srand((unsigned int) time(NULL));
    
    // Fill the entire buffer with random printable ASCII characters (from ' ' to '~').
    for (size_t i = 0; i < total_size-1; i++) {
        buffer[i] = 'a' + (rand() % ('z' - 'a' + 1));
    }
    buffer[total_size-1]=0;

    //send the cap for str
    cc_dcap sent_cap_str=cc_create_signed_cap_on_creg0(buffer, 0, total_size, true);
    //printf("Sender: ");
    //cc_print_cap(sent_cap_str);

    //start_t = clock();
    //MAC to be signed by kernel within our custom write_cap function
    cap_write(STDOUT_FILENO, &sent_cap_str);
    //write(STDOUT_FILENO, &sent_cap_str, sizeof(cc_dcap));
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    //printf("\nTotal time (s) of data write (%d MB):\t %f\n", (total_size),total_t);
    
    sleep(15);
    free(buffer);
    return EXIT_SUCCESS;
}
