#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include "cc_header.h"

clock_t start_t, end_t;
double total_t;

int main(int argc, char *argv[]){
    int total_size=0;
    if (argc > 1) {
        total_size = atol(argv[1])*KB_size;
        if (total_size == 0) {
            fprintf(stderr, "Invalid size provided.\n");
            return EXIT_FAILURE;
        }
    }
    // Allocate a single contiguous block of memory for the entire data.
    char *sent_buffer = (char*)malloc(total_size*sizeof(char));
    if (!sent_buffer) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    
    // Seed the random number generator.
    srand((unsigned int) time(NULL));
    
    // Fill the entire buffer with random printable ASCII characters (from ' ' to '~').
    for (size_t i = 0; i < total_size-1; i++) {
        sent_buffer[i] = 'a' + (rand() % ('z' - 'a' + 1));
    }
    sent_buffer[total_size-1]=0;
    
    // At this point, the entire data is ready in memory.
    // Now write the entire buffer in one syscall call.

    cc_dcap sent_cap_str=cc_create_signed_cap_on_creg0(sent_buffer, 0, total_size, false);
    write(STDOUT_FILENO, &sent_cap_str, sizeof(cc_dcap));
    close(STDOUT_FILENO); // Close stdout (write-end of pipe)
    //write(STDOUT_FILENO, sent_buffer, total_size+1);
    //printf("%d\n",total_size);
    //printf("%s\n",sent_buffer);
    sleep(40);
    free(sent_buffer);
    return 0;
}
