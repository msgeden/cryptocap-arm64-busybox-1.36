#include <stdio.h>
#include <unistd.h>

#include <sys/syscall.h>
#include <sys/wait.h>

#include "cc_header.h"

char global_char='G';
void foo(void)      
{   
    //global_char++; there is a problem if copy-on write is required for global_char 
    char local_char='L';
    asm volatile (
        ".word 0x02200d20\n\t"  //cldg8 x9, [cr0] (operand_1) 
        "mov x0, x9\n\t"          
        "mov x1, %0\n\t"          
        "mov x2, %1\n\t"          
        :
        : "r" (global_char), "r" (local_char)  // Output operands for x9
        :"x0", "x1", "x2", "x9", "memory"
    );
    cc_dret();
}

int main(void)
{
    int pipe_fd[2];  // [0] for reading, [1] for writing   

    // Create the pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t fpid = fork();
    
    if (fpid == 0) {

        // app process
        close(pipe_fd[1]);  // Close the write end of the pipe
        // Execute the Caller-Data Owner program
        dup2(pipe_fd[0], STDIN_FILENO);  // Redirect pipe read end to stdin
        execl("./caller.exe", "caller.exe", NULL);
        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);

    } else {

        // library process
        close(pipe_fd[0]);  // close the read end of the pipe
        
        // Allocate stack for caller process
        char *heap_stack = malloc(STACK_SIZE);
        if (!heap_stack) {
            perror("malloc");
            return EXIT_FAILURE;
        }
        
        // share the necessary info with the application process  
        cc_dcl call_grant=cc_grant_signed_DCLC(&foo, heap_stack+STACK_SIZE-16);
        printf("Callee:\t"); cc_print_dcl(call_grant);

        // Write the necessary call capability info to the pipe
        write(pipe_fd[1], &call_grant, sizeof(cc_dcl));
        
        printf("Callee: suspending this process:\n");
        cc_suspend_process(getpid());
        //wait(NULL);
        
        free(heap_stack);
    }
    return 0;
}