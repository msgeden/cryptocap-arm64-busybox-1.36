#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "cc_header.h"
#define BUFFER_SIZE 100
size_t total_size=0;

int main()  {

    int pipefd[2];
   
    pid_t pid;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    uint64_t int_via_cap = 52;

    pid = fork();
    if (pid == 0) { // Child process
       
        close(pipefd[1]); 
        
        cc_dcap recv_cap_stack_int;
        read(pipefd[0], &recv_cap_stack_int, sizeof(cc_dcap));
        printf("Child: "); cc_print_cap(recv_cap_stack_int);

        int recv_stack_int = cc_load_creg0_read_i64_data(recv_cap_stack_int);
        printf("Child: Integer received via piped-cap: %d\n", recv_stack_int);

        close(pipefd[0]);

    } else { // Parent process

        close(pipefd[0]);
        
        int_via_cap=99;
        //send the cap for int
        cc_dcap sent_cap_int=cc_create_signed_cap_on_creg0(&int_via_cap, 0, sizeof(uint64_t), false);
        printf("Parent: "); cc_print_cap(sent_cap_int);
        write(pipefd[1], &sent_cap_int, sizeof(cc_dcap));

        uint64_t local_stack_int = cc_load_creg0_read_i64_data(sent_cap_int);
        printf("Parent: Integer received via local-cap: %ld\n", local_stack_int);

        close(pipefd[1]);
        
        wait(NULL); // Wait for the child process to finish
    }

    return 0;
}