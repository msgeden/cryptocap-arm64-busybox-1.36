#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "cc_header.h"

#define BUFFER_SIZE 100
size_t total_size=1;

int main(int argc, char *argv[])  {

    if (argc > 1) {
        total_size = atol(argv[1])*KB_size;
        if (total_size == 0) {
            fprintf(stderr, "Invalid size provided.\n");
            return EXIT_FAILURE;
        }
    }
    
    
    int pipefd[2];
   
    pid_t pid;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();
    if (pid == 0) { // Child process
       
        close(pipefd[1]); 
        
        clock_t start_t, end_t;
        double total_t;
        
        cc_dcap recv_cap_str;
        read(pipefd[0], &recv_cap_str, sizeof(cc_dcap));
        printf("Receiver: "); cc_print_cap(recv_cap_str);

        start_t = clock();
        char* recv_copy=malloc(recv_cap_str.size);
        cc_memcpy_i8_asm(recv_copy, recv_cap_str, recv_cap_str.size);
        end_t = clock();
        total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;  
        //printf("Receiver: String received via pipe-cap: %s\n", copied_str);
        printf("Receiver: cross-domain memcpy via cldg instruction (at byte-level) is completed for %.2f of data (MB) in %.2f seconds\n", (double)recv_cap_str.size/(1024*1024), total_t);
        free(recv_copy);

        cc_resume_process(getppid());
        close(pipefd[0]);

    } else { // Parent process

        close(pipefd[0]);

        clock_t start_t, end_t;
        double total_t;
        
        char *buffer = malloc(total_size);
       
        if (!buffer) {
            perror("malloc");
            return EXIT_FAILURE;
        }
        
        // Seed the random number generator.
        srand((unsigned int) time(NULL));
        
        // Fill the entire buffer with random printable ASCII characters (from ' ' to '~').
        for (size_t i = 0; i < total_size-1; i++) {
            //buffer[i] = 'a' + (rand() % ('z' - 'a' + 1));
            buffer[i] = 'a' + (i % 27);

        }
        buffer[total_size - 1] = '\0'; // Null-terminate the string.


        cc_dcap sent_cap_str=cc_create_signed_cap_on_creg0(buffer, 0, total_size, false);
        printf("Sender: ");
        cc_print_cap(sent_cap_str);
        
        start_t = clock();
        char* local=malloc(sent_cap_str.size);
        ncc_memcpy_i8_asm(local, buffer, total_size);
        end_t = clock();
        total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;  
        //printf("Receiver: String received via pipe-cap: %s\n", copied_str);
        printf("Sender: intra-domain memcpy via conventional ldrb instruction (at byte-level) is completed for %.2f of data (MB) in %.2f seconds\n", (double)sent_cap_str.size/(1024*1024), total_t);
    
        

        start_t = clock();
        char* local_copy=malloc(sent_cap_str.size);
        cc_memcpy_i8_asm(local_copy, sent_cap_str, sent_cap_str.size);
        end_t = clock();
        total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;  
        //printf("Receiver: String received via pipe-cap: %s\n", copied_str);
        printf("Sender: intra-domain memcpy via cldg instruction (at byte-level) is completed for %.2f of data (MB) in %.2f seconds\n", (double)sent_cap_str.size/(1024*1024), total_t);
    
        

        write(pipefd[1], &sent_cap_str, sizeof(cc_dcap));

        cc_suspend_process(getpid());
        close(pipefd[1]);
        wait(NULL); // Wait for the child process to finish
        free(local);
        free(local_copy);

    }

    return 0;
}