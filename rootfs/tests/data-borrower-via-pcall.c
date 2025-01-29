#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "common.h"

int global_var=250;

__attribute__((naked)) 
void add_operands() {
    //TODO: decide whether/which callee-owned data should be accessed by functions called on behalf of caller

    // //Read operands from caller's domain
    asm volatile (
        ".word 0x02200120\n\t"  //cldg x9, [cr0] (operand_1) 
        ".word 0x02200141\n\t" //cldg x10, [cr1] (operand_2)
        :
            :
        :"x9","x10"
    );
    printf("Callee-Data Borrower: add_operands is called\n");

    // //Test memory access a callee-hosted variable (SP) (TODO: SOMETIME WORKS, SOMETIMES HANGS)
    // asm volatile (  
    //   "mov x11, sp\n\t"  
    //   "ldr x12, [x11]\n\t"        
    // );
    // asm volatile (
    //     "add x10, x10, x12\n\t"
    // );

    // //Test memory access a callee-hosted global variable (SP) (TODO: SOMETIME WORKS, SOMETIMES HANGS)
    // asm volatile (  
    //     "adrp x11, global_var\n\t"
    //     "add x11, x11, :lo12:global_var\n\t"
    //     "ldr x12, [x11]\n\t"        
    //     :
    //     :
    //     : "x11","x12"
    // );
    // asm volatile (
    //     "add x10, x10, x12\n\t"
    // );

    //Add two caller-hosted operand values
    asm volatile (
        "add x0, x9, x10\n\t"
    );

    //Return to the caller (cret)
    asm volatile (
     "mov x8, #467\t\n"
     "svc #0\n" 
    );
}

int main() {
    int pipe_fd[2];  // [0] for reading, [1] for writing

    // Create the pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork a child process to simulate the Caller
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // Child process: Caller-Data Owner
        
        close(pipe_fd[1]);  // Close the write end of the pipe

        // Execute the Caller-Data Owner program
        dup2(pipe_fd[0], STDIN_FILENO);  // Redirect pipe read end to stdin
        execl("./data-owner-via-pcall.exe", "data-owner-via-pcall.exe", NULL);
        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);

    } 
    else {  // Parent process: Callee-Data Borrower
        
        // int sum=0;
        // //Return to the caller (cret)
        // asm volatile (
        // "mov x0, #463\t\n"
        // "mov x8, #463\t\n"
        // "svc #0\n" 
        // );
            
        // asm volatile (
        // "mov %0, x0"   // Save the result from x0 into variable 'sum'
        //     : "=r" (sum)        // Output operand
        //     :                   // No input operands
        //     : "x0"        // Clobbered registers
        // );

        // // //the callee's cret via cjmp should jump to here. 
        // printf("Callee-Data Borrower: Successful return from 'cdummy' syscall with return value: %d\n",sum);
        

        close(pipe_fd[0]);  // Close the read end of the pipe
        int spid=getpid();
        printf("Callee-Data Borrower: PID: %d\n", spid);
        
        pdom code_cap;
        // Set up necessary context to let the caller execute  
        code_cap.PC = (uint64_t)&add_operands;  
        code_cap.SP = ((uint64_t)malloc(4096));      // Safe custom stack pointer (SP) pointing heap buffer
        code_cap.SP+=2048;                           
        code_cap.PT = getPTBase();
        code_cap.MAC = 0;
        code_cap.PID = spid;
        //code_cap.MAC= getKernelPTBase();
        //code_cap.MAC= getKernelSP();
    
        *((uint64_t*)code_cap.SP)=500;        
        // Write the necessary callee info (PC, SP, and TTBR0, PID) to the pipe
        write(pipe_fd[1], &code_cap, sizeof(pdom));
        
        printf("Callee-Data Borrower: Sent PC value: 0x%lx\n", code_cap.PC);
        printf("Callee-Data Borrower: Sent SP value: 0x%lx\n", code_cap.SP);
        printf("Callee-Data Borrower: Sent PT base register (TTBR0_EL1) value: 0x%lx\n", code_cap.PT);
        printf("Callee-Data Borrower: Sent PID value: 0x%lx\n", code_cap.PID);

        //printf("Callee-Data Borrower: Sent PT base register (TTBR1_EL1) value via MAC field: 0x%lx\n********************************************\n", code_cap.MAC);
        //printf("Callee-Data Borrower: Sent SP_EL1 value via MAC field: 0x%lx\n********************************************\n", code_cap.MAC);
        //printf("Callee-Data Borrower: Sent MAC value: 0x%lx\n********************************************\n", code_cap.MAC);
        printf("Callee-Data Borrower: Callee-hosted (HEAP) Operand-3:%ld\n", *((uint64_t*)code_cap.SP));
      
        //Test memory access to global_var via asm
        asm volatile (  
            "adrp x11, global_var\n\t"
            "add x11, x11, :lo12:global_var\n\t"
            "mov x12, #200\n\t"
            "str x12, [x11]\n\t"   
            :
            :
            :"x11","x12"     
        );

        printf("Callee-Data Borrower: Callee-hosted (GLOBAL) Operand-4:%d\n", global_var);

        printf("***********************************************************\n");

        // Close the pipe write end
        close(pipe_fd[1]);
        // Keep process alive and waiting for `pcall` to set the PC to add_operands
        //wait(NULL);
        // Sleep to allow the receiver to process
        sleep(5);
    }

    return 0;
}
