#include <stdio.h>
#include <unistd.h>

#include <sys/syscall.h>
#include <sys/wait.h>

#include "dcl-common.h"
__attribute__((naked))
void lib1_add(void)      
{
    
    //printf("Lib1: Lib1 is called\n");

    // int sum=100;
    // // int app_operand1=0;
    // int app_operand2=0;
    
    // asm volatile (
    //     ".word 0x02200120\n\t"  //cldg x9, [cr0] (operand_1) 
    //     "str x9, %0\n\t"          // Store x9 into memory at address of a
    //     ".word 0x02200141\n\t" //cldg x10, [cr1] (operand_2)
    //     "str x10, %1\n\t"          // Store x10 into memory at address of a
    //     :
    //     : "m" (app_operand1), "m" (app_operand2)   // Output operands for x9 and x10
    //     :"x9","x10"
    // );

    //sum=app_operand1+app_operand2+lib1_operand;
    //printf("Lib2: Lib2 returning to Lib1 with value:%d\n", sum);

    asm volatile(
        //"mov x0, %0\n\t"          // Load sum into x0
        "mov x0, #42\n\t"      // dret
        ".word 0x3e00000\n\t"      // dret
        //:
        //: "r"(sum) // Input operands
        //: "x0" // Clobbered registers
    );
    
    //exit(0);
}

int main(void)
{
    int pipe_fd[2];  // [0] for reading, [1] for writing   
    
    /////////////////////////////PREPARE TARGET REGISTER FOR CCALL /////////////////////////////#
    printf("Lib1: PID: %d\n", getpid());

    // Create the pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t fpid = fork();
    
    if (fpid == 0) {

        // Application process
        close(pipe_fd[1]);  // Close the write end of the pipe

        // Execute the Caller-Data Owner program
        dup2(pipe_fd[0], STDIN_FILENO);  // Redirect pipe read end to stdin
        execl("./app-dcall-dret.exe", "app-dcall-dret.exe", NULL);
        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);

    } else {
        // library process
        close(pipe_fd[0]);  // close the read end of the pipe
        
        int lib1_pid=getpid();
        uint64_t SP_EL0;
        
        // share the necessary info with the application process  
        dcapcl lib1_cap;
        lib1_cap.PC = (uint64_t)&lib1_add;  
        lib1_cap.SP_EL0 = ((uint64_t)malloc(4096));      // Safe custom stack pointer (SP) pointing heap buffer
        lib1_cap.SP_EL0+=2048;  
        printf("Lib1: HEAP SP: 0x%lx\n", lib1_cap.SP_EL0);        

        //fill a cdcapcl register via dgrant syscall
        syscall(SYS_agrant, lib1_cap.PC, lib1_cap.SP_EL0);
     
         // Store DCLC register to the lib1_cap
        asm volatile (
            "mov x9, %0\n\t"
            ".word 0x3a00089\n\t"      // cstcl dclc, x9
            :
            :"r"(&lib1_cap)
            :"x9" // clobber list 
        ); 

        // Write the necessary callee info (PC, SP, and TTBR0) to the pipe
        write(pipe_fd[1], &lib1_cap, sizeof(dcapcl));

        printf("Lib1: PID: %d\n", lib1_pid);
        printf("Lib1: Sent PC: 0x%lx\n", lib1_cap.PC);
        printf("Lib1: Sent SP: 0x%lx\n", lib1_cap.SP_EL0);        
        printf("Lib1: Sent TTBR0_EL1: 0x%lx\n", lib1_cap.TTBR0_EL1);
        printf("Lib1: Sent TTBR1_EL1: 0x%lx\n", lib1_cap.TTBR1_EL1);
        printf("Lib1: Sent TPIDR_EL1 (CURR): 0x%lx\n", lib1_cap.TASK_STRUCT);
        printf("Lib1: Sent TPIDR_EL0: 0x%lx\n", lib1_cap.TPIDR_EL0);
        printf("Lib1: Sent TPIDRRO_EL0/SP_EL1: 0x%lx\n", lib1_cap.TPIDRRO_EL0);

        wait_for_call_via_loop();    

    }

    return 0;
}