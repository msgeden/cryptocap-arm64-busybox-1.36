#include <stdio.h>
#include <unistd.h>

#include <sys/syscall.h>
#include <sys/wait.h>

#include "cc_header.h"
__attribute__((naked))
void foo(void)      
{   
    asm volatile (
        "mov x0, #65\n\t"          
        "mov x1, #66\n\t"          
        "mov x2, #67\n\t"     
        ".word 0x3e00000\n\t"      // dret
     
        :
        : 
        :"x0", "x1", "x2"
    );
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
        execl("./caller-simple.exe", "caller-simple.exe", NULL);
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
        
        //printf("Callee: suspending this process:\n");
        //cc_suspend_process(getpid());
        //wait(NULL);
        sleep(10);
        free(heap_stack);
    }
    return 0;
}