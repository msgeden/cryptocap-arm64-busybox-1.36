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

    char char_via_cap = 'A';

    pid = fork();
    if (pid == 0) { // Child process
       
        close(pipefd[1]); 

        printf("Child: Char read via before cross-domain access: %c\n", char_via_cap);


        cc_dcap recv_cap_stack_char;
        read(pipefd[0], &recv_cap_stack_char, sizeof(cc_dcap));
        printf("Child: "); cc_print_cap(recv_cap_stack_char);

        char recv_stack_char = cc_load_creg0_read_i8_data(recv_cap_stack_char);
        printf("Child: Char received via piped-cap: %c\n", recv_stack_char);

        char recv_stack_char_2 = cc_load_creg0_read_i8_data(recv_cap_stack_char);
        printf("Child: Char received via piped-cap: %c\n", recv_stack_char_2);

        printf("Child: Char read via after cross-domain access: %c\n", char_via_cap);

        close(pipefd[0]);

    } else { // Parent process

        close(pipefd[0]);
        
        char_via_cap='Z';
        //send the cap for int
        cc_dcap sent_cap_char=cc_create_signed_cap_on_creg0(&char_via_cap, 0, sizeof(char), false);
        printf("Parent: "); cc_print_cap(sent_cap_char);
        write(pipefd[1], &sent_cap_char, sizeof(cc_dcap));

        char local_stack_char = cc_load_creg0_read_i8_data(sent_cap_char);
        printf("Parent: Char received via local-cap: %c\n", local_stack_char);

        close(pipefd[1]);
        
        wait(NULL); // Wait for the child process to finish
    }

    return 0;
}