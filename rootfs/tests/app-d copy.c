#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include <sys/syscall.h>
#include <sys/wait.h>

#include "dcommon-412.h"
dcapcl lib_cap;

int main(void)
{
    //uint64_t PT_base=getPTBase();
    int app_pid=getpid();

    read(STDIN_FILENO, &lib_cap, sizeof(dcapcl));
    
  
    /////////////////////////////PREPARE TARGET REGISTER FOR DCALL /////////////////////////////#
    printf("App: PID: %d\n", app_pid);
    printf("App: Received PC=0x%lx, SP=0x%lx, TTBR0_EL1=0x%lx, TTBR1_EL1=0x%lx, CURRENT_TASK_STRUCT=0x%lx, PSTATE=0x%lx, TPIDR_EL0=0x%lx, TPIDRRO_EL0=0x%lx, FIELD_9=0x%lx, FIELD_10=0x%lx, FIELD_11=0x%lx, FIELD_12=0x%lx\n", lib_cap.PC,  lib_cap.SP_EL0, lib_cap.TTBR0_EL1, lib_cap.TTBR1_EL1, lib_cap.TASK_STRUCT, lib_cap.PSTATE, lib_cap.TPIDR_EL0, lib_cap.TPIDRRO_EL0, lib_cap.FIELD_9, lib_cap.FIELD_10, lib_cap.FIELD_11, lib_cap.FIELD_12);
       
    long sum=52;
    printf("App calling Lib via dcall\n");

    // Load DCLC register fields from lib_cap
    asm volatile (
        "mov x9, %0\n\t"
        ".word 0x3900089\n\t"      // cldcl dclc, x9
        :
        :"r"(&lib_cap)
        : "x9" // clobber list 
    ); 
    
    printf("App: Set PC: 0x%lx\n", lib_cap.PC);
    printf("App: Set SP: 0x%lx\n", lib_cap.SP_EL0);        
    printf("App: Set TTBR0_EL1: 0x%lx\n", lib_cap.TTBR0_EL1);
    printf("App: Set TTBR1_EL1: 0x%lx\n", lib_cap.TTBR1_EL1);
    printf("App: Set TASK_STRUCT: 0x%lx\n", lib_cap.TASK_STRUCT);
    printf("App: Set PSTATE: 0x%lx\n", lib_cap.PSTATE);
    printf("App: Set TPIDR_EL0: 0x%lx\n", lib_cap.TPIDR_EL0);
    printf("App: Set TPIDRRO_EL0: 0x%lx\n", lib_cap.TPIDRRO_EL0);
    printf("App: Set TCR_EL1: 0x%lx\n", lib_cap.FIELD_9);
    printf("App: Set SCTLR_EL1: 0x%lx\n", lib_cap.FIELD_10);
    printf("App: Set MAIR_EL1: 0x%lx\n", lib_cap.FIELD_11);

    //dcall
    asm volatile (
        //"str	x0,  [sp, #-8]!\n\t"
        // "str	x1,  [sp, #-8]!\n\t"
        // "str	x2,  [sp, #-8]!\n\t"
        // "str	x3,  [sp, #-8]!\n\t"
        // "str	x4,  [sp, #-8]!\n\t"
        // "str	x5,  [sp, #-8]!\n\t"
        // "str	x6,  [sp, #-8]!\n\t"
        // "str	x7,  [sp, #-8]!\n\t"
        // "str	x8,  [sp, #-8]!\n\t"
        "str	x9,  [sp, #-8]!\n\t"
        "str	x10, [sp, #-8]!\n\t"
        "str	x11, [sp, #-8]!\n\t"
        "str	x12, [sp, #-8]!\n\t"
        "str	x13, [sp, #-8]!\n\t"
        "str	x14, [sp, #-8]!\n\t"
        "str	x15, [sp, #-8]!\n\t"
        "str	x16, [sp, #-8]!\n\t"
        "str	x17, [sp, #-8]!\n\t"
        "str	x18, [sp, #-8]!\n\t"
        "str	x19, [sp, #-8]!\n\t"
        "str	x20, [sp, #-8]!\n\t"
        "str	x21, [sp, #-8]!\n\t"
        "str	x22, [sp, #-8]!\n\t"
        "str	x23, [sp, #-8]!\n\t"
        "str	x24, [sp, #-8]!\n\t"
        "str	x25, [sp, #-8]!\n\t"
        "str	x26, [sp, #-8]!\n\t"
        "str	x27, [sp, #-8]!\n\t"
        "str	x28, [sp, #-8]!\n\t"
        "str	x29, [sp, #-8]!\n\t"
        "str	x30, [sp, #-8]!\n\t"
        //"str	x31, [sp, #-8]!\n\t"
    );
    asm volatile(".word 0x3500000\n\t");     //DUMMY CJMP
    asm volatile(
        ".word 0x3d00000\n\t"      // dcall
    );
    asm volatile(".word 0x3500000\n\t");     //DUMMY CJMP
    asm volatile (
        
        //"ldr x31,  [sp], #8\n\t"  
        "ldr x30,  [sp], #8\n\t"  
        "ldr x29,  [sp], #8\n\t"  
        "ldr x28,  [sp], #8\n\t"  
        "ldr x27,  [sp], #8\n\t"  
        "ldr x26,  [sp], #8\n\t"  
        "ldr x25,  [sp], #8\n\t"  
        "ldr x24,  [sp], #8\n\t"  
        "ldr x23,  [sp], #8\n\t"  
        "ldr x22,  [sp], #8\n\t"  
        "ldr x21,  [sp], #8\n\t"  
        "ldr x20,  [sp], #8\n\t"  
        "ldr x19,  [sp], #8\n\t"  
        "ldr x18,  [sp], #8\n\t"  
        "ldr x17,  [sp], #8\n\t"  
        "ldr x16,  [sp], #8\n\t"  
        "ldr x15,  [sp], #8\n\t"  
        "ldr x14,  [sp], #8\n\t"  
        "ldr x13,  [sp], #8\n\t"  
        "ldr x12,  [sp], #8\n\t"  
        "ldr x11,  [sp], #8\n\t"  
        "ldr x10,  [sp], #8\n\t"  
        "ldr x9,   [sp], #8\n\t"  
        // "ldr x8,   [sp], #8\n\t"  
        // "ldr x7,   [sp], #8\n\t"  
        // "ldr x6,   [sp], #8\n\t"  
        // "ldr x5,   [sp], #8\n\t"  
        // "ldr x4,   [sp], #8\n\t"  
        // "ldr x3,   [sp], #8\n\t"  
        // "ldr x2,   [sp], #8\n\t"  
        // "ldr x1,   [sp], #8\n\t"  
        //"ldr x0,   [sp], #8\n\t"  
        //"ret\n\t"
    );

    asm volatile (
    "mov %0, x0"   // Save the result from x0 into variable 'sum'
        : "=r" (sum)        // Output operand
        :                   // No input operands
        : "x0"        // Clobbered registers
    );
    
    printf("App: Lib returned with result:%ld\n",sum);

    //exit(0);
    return 0;
}
