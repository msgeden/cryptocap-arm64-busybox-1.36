#include <stdio.h>
#include <unistd.h>

#include <sys/syscall.h>
#include <sys/wait.h>

#include "dcommon-412.h"
int lib1_operand=100;
dcapcl lib1_process_cap;
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
    
    /////////////////////////////PREPARE TARGET REGISTER FOR CCALL /////////////////////////////#
    printf("Lib1: PID: %d\n", getpid());
    int sum=50;
        
        
    // share the necessary info with the application process  
    dcapcl lib1_cap;
    lib1_cap.PC = (uint64_t)&lib1_add;  
    lib1_cap.SP_EL0 = ((uint64_t)malloc(4096));      // Safe custom stack pointer (SP) pointing heap buffer
    lib1_cap.SP_EL0+=2048;  
    printf("Lib1: HEAP SP: 0x%lx\n", lib1_cap.SP_EL0);        

    //fill a cdcapcl register via dgrant syscall
    syscall(SYS_dgrant, lib1_cap.PC, lib1_cap.SP_EL0);
    
    // Store DCLC register to the lib1_cap
    asm volatile (
            "mov x9, %0\n\t"
            ".word 0x3a00089\n\t"      // cstcl dclc, x9
            :
            :"r"(&lib1_cap)
            :"x9" // clobber list 
    ); 

    // Set DCLC register fields via lib1_procss_cap
    asm volatile (
        "mov x9, %0\n\t"
        ".word 0x3a00089\n\t"      // cstcl dclc, x9
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
    printf("App1: Set TPIDR: 0x%lx\n", lib1_process_cap.TPIDR_EL0);
    printf("App1: Set TPIDRRO/SP_EL1: 0x%lx\n", lib1_process_cap.TPIDRRO_EL0);

    //struct gpr_ctx ctx;
    //push_all_gprs();
    // asm volatile (
    //     // Reserve 256 bytes on the stack
    //     "sub     sp, sp, #256\n\t"
    //     // Store pairs: (x0, x1) at offset 0, (x2, x3) at 16, ... up to (x30, xzr) at 240
    //     "str	x30, [sp, #8]\n\t"
    //     "str	x29, [sp, #16]\n\t"
    //     "str	x28, [sp, #24]\n\t"
    //     "str	x27, [sp, #32]\n\t"
    //     "str	x26, [sp, #40]\n\t"
    //     "str	x25, [sp, #48]\n\t"
    //     "str	x24, [sp, #56]\n\t"
    //     "str	x23, [sp, #64]\n\t"
    //     "str	x22, [sp, #72]\n\t"
    //     "str	x21, [sp, #80]\n\t"
    //     "str	x20, [sp, #88]\n\t"
    //     "str	x19, [sp, #96]\n\t"
    //     "str	x18, [sp, #104]\n\t"
    //     "str	x17, [sp, #112]\n\t"
    //     "str	x16, [sp, #120]\n\t"
    //     "str	x15, [sp, #128]\n\t"
    //     "str	x14, [sp, #136]\n\t"
    //     "str	x13, [sp, #144]\n\t"
    //     "str	x12, [sp, #152]\n\t"
    //     "str	x11, [sp, #160]\n\t"    
    //     "str	x10, [sp, #168]\n\t"
    //     "str	x9, [sp, #176]\n\t"
    //     "str	x8, [sp, #184]\n\t"
    //     "str	x7, [sp, #192]\n\t"
    //     "str	x6, [sp, #200]\n\t"
    //     "str	x5, [sp, #208]\n\t"
    //     "str	x4, [sp, #216]\n\t"
    //     "str	x3, [sp, #224]\n\t"
    //     "str	x2, [sp, #232]\n\t"
    //     "str	x1, [sp, #240]\n\t"
    //     "str	x0, [sp, #248]\n\t"
    // );
    // push_all_gprs();
    // asm volatile(
    //     ".word 0x3d00000\n\t"      // dcall
    // );
    // pop_all_gprs();

    // asm volatile (
    //     "ldr x0, [sp, #248]\n\t"
    //     "ldr x1, [sp, #240]\n\t"
    //     "ldr x2, [sp, #232]\n\t"
    //     "ldr x3, [sp, #224]\n\t"
    //     "ldr x4, [sp, #216]\n\t"
    //     "ldr x5, [sp, #208]\n\t"
    //     "ldr x6, [sp, #200]\n\t"
    //     "ldr x7, [sp, #192]\n\t"
    //     "ldr x8, [sp, #184]\n\t"
    //     "ldr x9, [sp, #176]\n\t"
    //     "ldr x10, [sp, #168]\n\t"
    //     "ldr x11, [sp, #160]\n\t"
    //     "ldr x12, [sp, #152]\n\t"
    //     "ldr x13, [sp, #144]\n\t"
    //     "ldr x14, [sp, #136]\n\t"
    //     "ldr x15, [sp, #128]\n\t"
    //     "ldr x16, [sp, #120]\n\t"
    //     "ldr x17, [sp, #112]\n\t"
    //     "ldr x18, [sp, #104]\n\t"
    //     "ldr x19, [sp, #96]\n\t"
    //     "ldr x20, [sp, #88]\n\t"
    //     "ldr x21, [sp, #80]\n\t"
    //     "ldr x22, [sp, #72]\n\t"
    //     "ldr x23, [sp, #64]\n\t"
    //     "ldr x24, [sp, #56]\n\t"
    //     "ldr x25, [sp, #48]\n\t"
    //     "ldr x26, [sp, #40]\n\t"
    //     "ldr x27, [sp, #32]\n\t"
    //     "ldr x28, [sp, #24]\n\t"
    //     "ldr x29, [sp, #16]\n\t"
    //     "ldr x30, [sp, #8]\n\t"
    //     "add     sp, sp, #256\n\t"
    // );
    dcall();
    asm volatile(
        "mov %0, x0\n\t"          // Move result from x0 to result
        : "=r"(sum)            // Output operand
        :
        : "x0"
    );
    printf("App: Lib1 returned with result:%d\n",sum);


    return 0;
}   