#include <stdio.h>
#include <unistd.h>

#include <sys/syscall.h>
#include <sys/wait.h>

#include "cc-common.h"
int lib1_operand=100;
pcap lib2_process_cap;

int lib1_add(void)      
{
   
    int app_operand1=0;
    int sum;
    asm volatile (
        ".word 0x02200120\n\t"  //cldg x9, [cr0] (operand_1) 
        "str x9, %0\n\t"          // Store x9 into memory at address of a
        :
        : "m" (app_operand1)   // Output operands for x9 and x10
        :"x9"
    ); 
    printf("Lib1: calling Lib2\n");

    // asm volatile(
    //     "mov x0, %1\n\t"          // Load PID into x0
    //     "mov x1, %2\n\t"          // Load PC into x1
    //     "mov x2, %3\n\t"          // Load MAC into x2
    //     ".word 0x3d00000\n\t"      // pcall
    //     "mov %0, x0\n\t"          // Move result from x0 to sum
    //     : "=r"(sum)            // Output operand
    //     : "r"(lib2_process_cap.PID), "r"(lib2_process_cap.PC), "r"(lib2_process_cap.MAC) // Input operands
    //     : "x0", "x1", "x2", "memory" // Clobbered registers
    // );
    sum=syscall(SYS_pcall, lib2_process_cap.PID, lib2_process_cap.PC, lib2_process_cap.MAC);
    
    printf("Lib1: Lib2 returned with result:%d\n",sum);
    sum=sum+lib1_operand+app_operand1;
    printf("Lib1: Lib1 returning to App with value:%d\n", sum);
    
    // asm volatile(
    //     "mov x0, %0\n\t"          // Load sum into x0
    //     ".word 0x3e00000\n\t"      // pret
    //     :
    //     : "r"(sum) // Input operands
    //     : "x0" // Clobbered registers
    // );
    syscall(SYS_pret, sum);
    exit(0);
}

int main(void)
{
    int pipe_fd[2];  // [0] for reading, [1] for writing   
    read(STDIN_FILENO, &lib2_process_cap, sizeof(pcap));
    
    /////////////////////////////PREPARE TARGET REGISTER FOR CCALL /////////////////////////////#
    printf("Lib1: PID: %d\n", getpid());
    printf("Lib1: Received PID=%ld, PC=0x%lx, MAC=0x%lx\n", lib2_process_cap.PID, lib2_process_cap.PC, lib2_process_cap.MAC);

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
        execl("./app-pcall-pret", "app-pcall-pret", NULL);
        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);

    } else {
        // library process
        close(pipe_fd[0]);  // close the read end of the pipe
        
        int lib1_pid=getpid();
        
        
        // share the necessary info with the application process  
        pcap lib1_cap;
        lib1_cap.PC = (uint64_t)&lib1_add;  
        lib1_cap.PID = lib1_pid;
        lib1_cap.MAC = 0;
     
        // Write the necessary callee info (PC, SP, and TTBR0) to the pipe
        write(pipe_fd[1], &lib1_cap, sizeof(pcap));

        printf("Lib1: PID: %d\n", lib1_pid);
        printf("Lib1: Sent Target PID: %ld\n", lib1_cap.PID);
        printf("Lib1: Sent Target Address: 0x%lx\n", lib1_cap.PC);
        printf("Lib1: Sent MAC value: 0x%lx\n", lib1_cap.MAC);
        
        wait_for_call_via_loop();    

     }

    return 0;
}