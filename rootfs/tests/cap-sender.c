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

size_t total_size=1;
clock_t start_t, end_t;
double total_t;
double sender_t;

int main(int argc, char *argv[]) {

    int pipefd[2];
    
    if (argc > 1) {
        total_size = atol(argv[1])*KB_size;
        if (total_size == 0) {
            fprintf(stderr, "Invalid size provided.\n");
            return EXIT_FAILURE;
        }
    }
    
    pid_t pid;

    char buffer[BUFFER_SIZE];
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();
    if (pid == 0) { // Child process

        close(pipefd[1]); 
        
        dup2(pipefd[0], STDIN_FILENO);  // Redirect pipe read end to stdin

        execl("./cap-receiver.exe", "cap-receiver.exe", NULL);
        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);
    
    } else { // Parent process

        close(pipefd[0]);
        
        // //send the text
        // char message_via_pipe[] = "Hello from sender via pipe-copy!";
        // write(pipefd[1], message_via_pipe, sizeof(buffer));

       
        // Allocate a single contiguous block of memory for the entire data.
        char *message_via_cap = malloc(total_size);

        if (!message_via_cap) {
            perror("malloc");
            return EXIT_FAILURE;
        }
        
        // // Seed the random number generator.
        srand((unsigned int) time(NULL));
        
        // Fill the entire buffer with random printable ASCII characters (from ' ' to '~').
        for (size_t i = 0; i < total_size-1; i++) {
            //message_via_cap[i] = 'a' + (rand() % ('z' - 'a' + 1));
            message_via_cap[i] = 'a' + (i % 27);

        }
        message_via_cap[total_size-1] = '\0';
        //printf("Sender: %s", message_via_cap);


        cc_dcap sent_cap_str=cc_create_signed_cap_on_creg0(message_via_cap, 0, total_size, false);
        printf("Sender: "); cc_print_cap(sent_cap_str);

        // start_t = clock();
        // char* local_copy1=malloc(sent_cap_str.size);
        // ncc_memcpy_i8_asm(local_copy1, buffer, total_size);
        // end_t = clock();
        // total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;  
        // //printf("Receiver: String received via pipe-cap: %s\n", copied_str);
        // printf("Sender: intra-domain memcpy via conventional ldrb instruction (at byte-level) is completed for %.2f of data (MB) in %.2f seconds\n", (double)sent_cap_str.size/(1024*1024), total_t);
    
        start_t = clock();
        char* local_copy2=malloc(sent_cap_str.size);
        cc_memcpy_i8_asm_new(local_copy2, sent_cap_str, sent_cap_str.size);
        end_t = clock();
        total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;  
        //printf("Sender: String received via pipe-cap: %s\n", copied_str);
        printf("Sender: intra-domain memcpy via cldg (at byte-level) is completed for %.2f of data (MB) in %.2f seconds\n", (double)sent_cap_str.size/(1024*1024), total_t);

        //syscall(SYS_write_cap, pipefd[1], &sent_cap_str);
        write(pipefd[1], &sent_cap_str, sizeof(cc_dcap));


        cc_suspend_process(getpid());
        wait(NULL); // Wait for the child process to finish

        close(pipefd[1]);
        
        free(message_via_cap);
        //free(local_copy1);
        free(local_copy2);
 
        
    }


    return 0;
}