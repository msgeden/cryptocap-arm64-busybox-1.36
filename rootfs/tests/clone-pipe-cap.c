#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sched.h>

#include "cc_header.h"

#define STACK_SIZE (1024 * 1024) // Stack size for child

size_t total_size = 1;
clock_t start_t, end_t;
double total_t;
double sender_t;

int pipefd[2];  // Pipe for communication

// Receiver function (runs in the cloned child process)
int receiver(void *arg) {
    close(pipefd[1]); // Close write end

    cc_dcap recv_cap_str;
    read(pipefd[0], &recv_cap_str, sizeof(cc_dcap));
    printf("Receiver: "); 
    cc_print_cap(recv_cap_str);

    start_t = clock();
    char* copied_str = malloc(recv_cap_str.size);
    cc_memcpy_i8_asm(copied_str, recv_cap_str, recv_cap_str.size);
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;  
    printf("Receiver: cross-domain memcpy via cldg instruction (at byte-level) is completed for %.2f MB in %.2f seconds\n", (double)recv_cap_str.size/(1024*1024), total_t);

    cc_resume_process(getppid()); // Resume sender

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        total_size = atol(argv[1]) * KB_size;
        if (total_size == 0) {
            fprintf(stderr, "Invalid size provided.\n");
            return EXIT_FAILURE;
        }
    }

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    // Allocate stack for child process
    char *child_stack = malloc(STACK_SIZE);
    if (!child_stack) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    pid_t pid = clone(receiver, child_stack + STACK_SIZE, SIGCHLD, NULL);
    if (pid == -1) {
        perror("clone");
        free(child_stack);
        return EXIT_FAILURE;
    }

    close(pipefd[0]); // Close read end in sender

    // Allocate data for sending
    char *message_via_cap = malloc(total_size);
    if (!message_via_cap) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    // Fill buffer with data
    for (size_t i = 0; i < total_size - 1; i++) {
        message_via_cap[i] = 'a' + (i % 27);
    }
    message_via_cap[total_size - 1] = '\0';

    cc_dcap sent_cap_str = cc_create_signed_cap_on_creg0(message_via_cap, 0, total_size, true);
    printf("Sender: "); 
    cc_print_cap(sent_cap_str);

    start_t = clock();
    char* copied_str = malloc(sent_cap_str.size);
    cc_memcpy_i8_asm(copied_str, sent_cap_str, sent_cap_str.size);
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;  
    printf("Sender: intra-domain memcpy via cldg (at byte-level) is completed for %.2f MB in %.2f seconds\n", (double)sent_cap_str.size/(1024*1024), total_t);

    write(pipefd[1], &sent_cap_str, sizeof(cc_dcap));

    cc_suspend_process(getpid()); // Suspend until receiver resumes it

    close(pipefd[1]); 
    waitpid(pid, NULL, 0); // Wait for receiver to finish

    free(message_via_cap);
    free(child_stack);

    return 0;
}
