#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include <sys/syscall.h>
#include <sys/wait.h>

#include "dcommon-412.h"
dcapcl lib1_process_cap;

//Data to be accessed by/added within Callee
uint64_t app_hosted_operand_1=411;
uint64_t app_hosted_operand_2=589;

int main(void)
{
    //uint64_t PT_base=getPTBase();
    int app_pid=getpid();

    read(STDIN_FILENO, &lib1_process_cap, sizeof(dcapcl));
    
    /////////////////////////////PREPARE TARGET REGISTER FOR DCALL /////////////////////////////#
    printf("App: PID: %d\n", app_pid);
    printf("App: Received PC=0x%lx, SP=0x%lx, TTBR0_EL1=0x%lx, TTBR1_EL1=0x%lx, CURRENT_TASK_STRUCT=0x%lx, PSTATE=0x%lx, TPIDR_EL0=0x%lx, TPIDRRO_EL0=0x%lx, FIELD_9=0x%lx, FIELD_10=0x%lx, FIELD_11=0x%lx, FIELD_12=0x%lx\n", lib1_process_cap.PC,  lib1_process_cap.SP_EL0, lib1_process_cap.TTBR0_EL1, lib1_process_cap.TTBR1_EL1, lib1_process_cap.TASK_STRUCT, lib1_process_cap.PSTATE, lib1_process_cap.TPIDR_EL0, lib1_process_cap.TPIDRRO_EL0, lib1_process_cap.FIELD_9, lib1_process_cap.FIELD_10, lib1_process_cap.FIELD_11, lib1_process_cap.FIELD_12);


    // /////////////////////////////PREPARE CAPABILITY REGISTER ARGUMENTS FOR CCALL/////////////////////////////#
  
    // printf("App: Setting CR0 and CR1 registers as function application-hosted arguments\n");
    // printf("App: Operand-1=%ld\n",app_hosted_operand_1);
    // printf("App: Operand-1=%ld\n",app_hosted_operand_2);
    
    // // // Set cr0.base via x9
    // asm volatile (
    //     "mov x9, %0\n\t"
    //     ".word 0x02500009\n\t"      // csetbase cr0, cr0, x9
    //     :
    //     :"r"(&app_hosted_operand_1)
    //     : "x9" // clobber list 
    // );

    // // Set cr0.perms (RWXT) via x9
    // __asm__ volatile (
    //     "mov x9, #0xF\n\t"       // mov x9, #0xF
    //     ".word 0x02700009\n\t"      // csetperms cr0, cr0, x9
    // );

    // // Set cr0.size info (8 bytes) via x9
    // __asm__ volatile (
    //     "mov x9, #0x8\n\t"   // mov x0, #0x00000008
    //     ".word 0x02600009\n\t"      // csetsize cr0, cr0, x9
    // );

    // // Set cr0.offset (0) via x9
    // __asm__ volatile (
    //     "mov x9, #0x0\n\t"   // mov x0, #0x0
    //     ".word 0x02800009\n\t"      // csetaddr cr1, cr0, x9
    // );

    // // Set cr0.PT via x9
    // __asm__ volatile (
    //     "mov x9, %0\n\t"           
    //     ".word 0x03400009\n\t"      // csetpt cr0, cr0, x9
    //     :                        // no output 
    //     : "r" (PT_base)          // input
    //     : "x9"                   // clobber list
    // );

    // //Move/Copy cr0 to cr1
    // __asm__ volatile (
    //     ".word 0x03000020\n\t"      // cmov   cr1, cr0
    // );

    // // Change base address (operand_1) of cr1 to the address of operand_2
    // asm volatile (
    //     "mov x9, %0\n\t"
    //     ".word 0x02500429\n\t"      // csetbase cr1, cr1, x9
    //     :
    //     :"r"(&app_hosted_operand_2)
    //     : "x9"            // clobber list 
    // );

    // ///////////////////////////PERFORM THE CALL/////////////////////////////#

       
    long sum=5;
    printf("App1: calling Lib1 via dcall\n");

    // Load DCLC register fields from lib1_procss_cap
    asm volatile (
        "mov x9, %0\n\t"
        ".word 0x3900089\n\t"      // cldcl dclc, x9
        :
        :"r"(&lib1_process_cap)
        : "x9" // clobber list 
    ); 
    
    printf("App1: Set PC: 0x%lx\n", lib1_process_cap.PC);
    printf("App1: Set SP: 0x%lx\n", lib1_process_cap.SP_EL0);        
    printf("App1: Set TTBR0_EL1: 0x%lx\n", lib1_process_cap.TTBR0_EL1);
    printf("App1: Set TTBR1_EL1: 0x%lx\n", lib1_process_cap.TTBR1_EL1);
    printf("App1: Set TASK_STRUCT: 0x%lx\n", lib1_process_cap.TASK_STRUCT);
    printf("App1: Set PSTATE: 0x%lx\n", lib1_process_cap.PSTATE);
    printf("App1: Set TPIDR_EL0: 0x%lx\n", lib1_process_cap.TPIDR_EL0);
    printf("App1: Set TPIDRRO_EL0: 0x%lx\n", lib1_process_cap.TPIDRRO_EL0);
    printf("App1: Set TCR_EL1: 0x%lx\n", lib1_process_cap.FIELD_9);
    printf("App1: Set SCTLR_EL1: 0x%lx\n", lib1_process_cap.FIELD_10);
    printf("App1: Set MAIR_EL1: 0x%lx\n", lib1_process_cap.FIELD_11);

    dcall();
    
    asm volatile (
    "mov %0, x0"   // Save the result from x0 into variable 'sum'
        : "=r" (sum)        // Output operand
        :                   // No input operands
        : "x0"        // Clobbered registers
    );
    
    printf("App: Lib1 returned with result:%ld\n",sum);

    //exit(0);
    return 0;
}
