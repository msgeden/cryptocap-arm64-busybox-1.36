#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>



#include "cc_header.h"

size_t total_size=0;
clock_t start_t, end_t;
double total_t;
double sender_t;

int main(int argc, char *argv[]) {

    int pipefd[2];
    
    if (argc < 2) {
        total_size = atol(argv[1])*KB_size;
        if (total_size == 0) {
            fprintf(stderr, "Invalid size provided.\n");
            return EXIT_FAILURE;
        }
    }

    char buffer[BUFFER_SIZE];
    pid_t pid;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();
    uint64_t int_via_cap = 52;
    if (pid == 0) { // Child process

        close(pipefd[1]); 
        
        dup2(pipefd[0], STDIN_FILENO);  // Redirect pipe read end to stdin

        execl("./pipe-receiver-412.exe", "pipe-receiver-412.exe", NULL);
        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);
    
    } else { // Parent process

        close(pipefd[0]);
        
        //send the text
        char message_via_pipe[] = "Hello from sender via pipe-copy!";

        write(pipefd[1], message_via_pipe, sizeof(buffer));

        //send the cap for str
                                                                                                                                                                                                                                                                    
                                                                                                                                                                                                                                                                    
        //char message_via_cap[] = "Hello from sender via pipe-cap!";
 
        // Allocate a single contiguous block of memory for the entire data.
        char *message_via_cap = malloc(total_size+1);
        printf("Sender: malloc address:0x%lx\n", message_via_cap);

        if (!buffer) {
            perror("malloc");
            return EXIT_FAILURE;
        }
        
        // Seed the random number generator.
        srand((unsigned int) time(NULL));
        
        // Fill the entire buffer with random printable ASCII characters (from ' ' to '~').
        for (size_t i = 0; i < total_size; i++) {
            message_via_cap[i] = 'a' + (rand() % ('z' - 'a' + 1));
        }
        //printf("Sender: %s", message_via_cap);


        cc_dcap sent_cap_str=cc_create_signed_cap_on_CR0(message_via_cap, 0, strlen(message_via_cap) + 1, true);
        printf("Sender: ");
        cc_print_cap(sent_cap_str);

        //send the cap for int
        cc_dcap sent_cap_int=cc_create_signed_cap_on_CR0(&int_via_cap, 0, sizeof(uint64_t), true);
        printf("Sender: ");
        cc_print_cap(sent_cap_int);
        
        int_via_cap++;
        
        uint64_t int_value = cc_load_CR0_read_i64_data(sent_cap_int);
        printf("Sender: Integer received via intra-domain cap: %ld\n", int_value);

        //MAC to be signed by kernel within our custom write_cap function
        //write(pipefd[1], &sent_cap_str, sizeof(cc_dcap));
        //write(pipefd[1], &sent_cap_int, sizeof(cc_dcap));
        syscall(SYS_write_cap, pipefd[1], &sent_cap_str);
        syscall(SYS_write_cap, pipefd[1], &sent_cap_int);

        printf("Sender: close\n");


        close(pipefd[1]);
       
        printf("Sender: wait\n");

        wait(NULL); // Wait for the child process to finish
        
        // //write_cap(pipefd[1], &sent_cap_str);

        // // --- Freeze this (sender) process---
        // cc_suspend_process(getpid());
        
        // char* local_copy=malloc(sent_cap_str.size);
        // printf("Sender: allocated heap (malloc) base for local memcpy:0x%p\n", local_copy);
        // start_t = clock();
        // cc_memcpy_i8(local_copy, sent_cap_str, sent_cap_str.size);
        // end_t = clock();
        // total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;    
        // printf("Sender: local memcpy is completed for %.2f of data (MB) in %.2f seconds\n", (double)sent_cap_str.size/(1024*1024), total_t);
    
        // Free the allocated memory
        //free(local_copy); 
        
    }

    printf("Return\n");

    return 0;
}