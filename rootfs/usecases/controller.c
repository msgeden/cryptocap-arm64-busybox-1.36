#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

#define SHM_KEY 0x1234
#define BUF_SIZE 1024
#define NUM_WORKERS 100
#define NUM_ITERATIONS 100000  // Number of attach/detach cycles per worker

// Function to get time in nanoseconds
long long current_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int main() {
    // Create a System V shared memory segment
    int shmid = shmget(SHM_KEY, BUF_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory in the controller process
    char *shared_data = (char*)shmat(shmid, NULL, 0);
    if (shared_data == (char*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Write some data to the shared memory
    strcpy(shared_data, "Hello, this is shared memory!");

    // Total time measurement for all workers
    long long total_time_ns = 0;

    printf("Starting microbenchmark with %d workers, each performing %d attach/detach cycles...\n",
           NUM_WORKERS, NUM_ITERATIONS);

    for (int i = 0; i < NUM_WORKERS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Worker process: execute worker binary
            char iterations[16];
            snprintf(iterations, sizeof(iterations), "%d", NUM_ITERATIONS);
            execlp("./worker", "./worker", iterations, NULL);
            perror("execlp");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all workers to finish and sum up their reported times
    for (int i = 0; i < NUM_WORKERS; i++) {
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            total_time_ns += WEXITSTATUS(status);
        }
    }

    // Detach shared memory in controller
    if (shmdt(shared_data) == -1) {
        perror("shmdt");
    }

    // Mark shared memory for deletion
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }

    printf("Total time for all workers: %lld ns\n", total_time_ns);
    printf("Average time per worker: %lld ns\n", total_time_ns / NUM_WORKERS);

    return 0;
}
