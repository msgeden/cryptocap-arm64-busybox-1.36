#include <stdio.h>
#include <unistd.h>

#include <sys/syscall.h>
#include <sys/wait.h>

#include "dcommon-412.h"

#define STACK_SIZE (1024 * 1024) // Stack size for child

__attribute__((naked))
void lib_add(void)      
{
    asm volatile(
         "mov x0, #42\n\t"      
         ".word 0x3e00000\n\t"      // dret
    );
}

int main(void)
{
    int pipe_fd[2];  // [0] for reading, [1] for writing   
    /////////////////////////////PREPARE TARGET REGISTER FOR CCALL /////////////////////////////#
    printf("Lib: PID: %d\n", getpid());

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
        execl("./app-d.exe", "app-d.exe", NULL);
        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);

    } else {

        // library process
        close(pipe_fd[0]);  // close the read end of the pipe
        
        int lib_pid=getpid();
        
        // Allocate stack for child process
        char *heap_stack = malloc(STACK_SIZE);
        if (!heap_stack) {
            perror("malloc");
            return EXIT_FAILURE;
        }
        printf("Lib: heap stack: %p\n", heap_stack);        

        // share the necessary info with the application process  
        dcapcl lib_cap;
        lib_cap.PC = (uint64_t)&lib_add;  
        lib_cap.SP_EL0 = (uint64_t)heap_stack+STACK_SIZE-16; // Safe custom stack pointer (SP) pointing heap buffer
        printf("Lib: HEAP END/STACK START: 0x%lx\n", lib_cap.SP_EL0);        

        //fill the DCLC capability call register via dgrant syscall
        syscall(SYS_dgrant, lib_cap.PC, lib_cap.SP_EL0);
     
         // Store value of DCLC register into lib_cap struct
        asm volatile (
            "mov x9, %0\n\t"
            ".word 0x3a00089\n\t"      // cstcl dclc, x9
            :
            :"r"(&lib_cap)
            :"x9" // clobber list 
        ); 


        printf("Lib: PID: %d\n", lib_pid);
        printf("Lib: Sent PC: 0x%lx\n", lib_cap.PC);
        printf("Lib: Sent SP (heap): 0x%lx\n", lib_cap.SP_EL0);        
        printf("Lib: Sent TTBR0_EL1: 0x%lx\n", lib_cap.TTBR0_EL1);
        printf("Lib: Sent TTBR1_EL1: 0x%lx\n", lib_cap.TTBR1_EL1);
        printf("Lib: Sent TPIDR_EL1 (CURR): 0x%lx\n", lib_cap.TASK_STRUCT);
        printf("Lib: Sent PSTATE/SPSR_EL1: 0x%lx\n", lib_cap.PSTATE);
        printf("Lib: Sent TPIDR_EL0: 0x%lx\n", lib_cap.TPIDR_EL0);
        printf("Lib: Sent TPIDRRO_EL0: 0x%lx\n", lib_cap.TPIDRRO_EL0);
        printf("Lib: Sent TCR_EL1: 0x%lx\n", lib_cap.FIELD_9);
        printf("Lib: Sent SCTLR_EL1: 0x%lx\n", lib_cap.FIELD_10);
        printf("Lib: Sent MAIR_EL1: 0x%lx\n", lib_cap.FIELD_11);

        // Write the necessary call capability info to the pipe
        write(pipe_fd[1], &lib_cap, sizeof(dcapcl));

        //wait_for_call_via_loop();   
        _
        wait(NULL);
        
        free(heap_stack);
    }
    return 0;
}