#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <time.h>

#define SHM_KEY 0x1234
#define BUF_SIZE 1024

// Function to get time in nanoseconds
long long current_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_iterations>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_iterations = atoi(argv[1]);

    // Start time for this worker
    long long start_time = current_time_ns();

    for (int i = 0; i < num_iterations; i++) {
        // Get the shared memory segment
        int shmid = shmget(SHM_KEY, BUF_SIZE, 0666);
        if (shmid < 0) {
            perror("worker shmget");
            exit(EXIT_FAILURE);
        }

        // Attach to the shared memory
        char *data = (char*)shmat(shmid, NULL, 0);
        if (data == (char*)-1) {
            perror("worker shmat");
            exit(EXIT_FAILURE);
        }

        // Simulate minimal access
        volatile char c = data[0];  // Dummy read to ensure memory is touched

        // Detach the shared memory
        if (shmdt(data) == -1) {
            perror("worker shmdt");
            exit(EXIT_FAILURE);
        }
    }

    // End time for this worker
    long long end_time = current_time_ns();

    // Compute and print the total time for this worker
    long long total_time_ns = end_time - start_time;
    printf("[Worker %d] Total time: %lld ns\n", getpid(), total_time_ns);

    // Return total time as exit code (truncated to 1 byte for simplicity)
    exit(total_time_ns % 256);
}
