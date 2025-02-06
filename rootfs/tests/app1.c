#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>

clock_t start_t, end_t;
double total_t;

int main(int argc, char *argv[]){
    int total_size=0;
    if (argc > 1) {
        total_size = atol(argv[1])*1024;
        if (total_size == 0) {
            fprintf(stderr, "Invalid size provided.\n");
            return EXIT_FAILURE;
        }
    }
    // Allocate a single contiguous block of memory for the entire data.
    char *buffer = (char*)malloc(total_size*sizeof(char));
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
    
    // At this point, the entire data is ready in memory.
    // Now write the entire buffer in one syscall call.

    start_t = clock();
    write(STDOUT_FILENO, &total_size, sizeof(int));
    write(STDOUT_FILENO, buffer, total_size);
    //printf("%d\n",total_size);
    //printf("%s\n",buffer);
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    wait(10);
    free(buffer);
    return 0;
}
