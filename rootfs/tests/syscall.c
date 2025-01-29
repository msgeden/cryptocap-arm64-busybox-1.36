#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <stdint.h>


// Function to get the TTBR0_EL1 register value (Page Table Base Register)
uint64_t get_ttbr0_el1() {
    uint64_t value=0xDEADBEEF;
    //asm volatile("mrs x0, ttbr0_el1" : "=r"(value)); 
    //read PT base register (TTBR0_EL1) value into x0  
    asm volatile(".word 0x03300000" : "=r"(value));  //readttbr x0
    printf("get_ttbr0_el1(): 0x%lx\n", value);
    return value;
}

// Function to print all general-purpose registers (GPRs) on ARM64 without modifying them
void print_gpr_values() {
    uint64_t x0, x1, x2, x3, x4, x5, x6, x7;
    uint64_t x8, x9, x10, x11, x12, x13, x14, x15;
    uint64_t x16, x17, x18, x19, x20, x21, x22, x23;
    uint64_t x24, x25, x26, x27, x28, x29, x30;

    // Save all GPRs onto the stack
    asm volatile (
        "sub sp, sp, #272\n"        // Make space on the stack for all GPRs (256 bytes for 32 registers)
        "stp x0, x1, [sp, #0]\n"    // Store x0 and x1
        "stp x2, x3, [sp, #16]\n"   // Store x2 and x3
        "stp x4, x5, [sp, #32]\n"   // Store x4 and x5
        "stp x6, x7, [sp, #48]\n"   // Store x6 and x7
        "stp x8, x9, [sp, #64]\n"   // Store x8 and x9
        "stp x10, x11, [sp, #80]\n" // Store x10 and x11
        "stp x12, x13, [sp, #96]\n" // Store x12 and x13
        "stp x14, x15, [sp, #112]\n"// Store x14 and x15
        "stp x16, x17, [sp, #128]\n"// Store x16 and x17
        "stp x18, x19, [sp, #144]\n"// Store x18 and x19
        "stp x20, x21, [sp, #160]\n"// Store x20 and x21
        "stp x22, x23, [sp, #176]\n"// Store x22 and x23
        "stp x24, x25, [sp, #192]\n"// Store x24 and x25
        "stp x26, x27, [sp, #208]\n"// Store x26 and x27
        "stp x28, x29, [sp, #224]\n"// Store x28 (FP) and x29 (LR)
        "str x30, [sp, #240]\n"     // Store x30 (LR)
   
    );

    // Load the register values from the stack into C variables
    asm volatile (
        "ldp %0, %1, [sp, #0]\n"
        "ldp %2, %3, [sp, #16]\n"
        : "=r"(x0), "=r"(x1), "=r"(x2), "=r"(x3)
    );
    
    asm volatile (
        "ldp %0, %1, [sp, #32]\n"
        "ldp %2, %3, [sp, #48]\n"
        : "=r"(x4), "=r"(x5), "=r"(x6), "=r"(x7)
    );

    asm volatile (
        "ldp %0, %1, [sp, #64]\n"
        "ldp %2, %3, [sp, #80]\n"
        : "=r"(x8), "=r"(x9), "=r"(x10), "=r"(x11)
    );

    asm volatile (
        "ldp %0, %1, [sp, #96]\n"
        "ldp %2, %3, [sp, #112]\n"
        : "=r"(x12), "=r"(x13), "=r"(x14), "=r"(x15)
    );

    asm volatile (
        "ldp %0, %1, [sp, #128]\n"
        "ldp %2, %3, [sp, #144]\n"
        : "=r"(x16), "=r"(x17), "=r"(x18), "=r"(x19)
    );

    asm volatile (
        "ldp %0, %1, [sp, #160]\n"
        "ldp %2, %3, [sp, #176]\n"
        : "=r"(x20), "=r"(x21), "=r"(x22), "=r"(x23)
    );

    asm volatile (
        "ldp %0, %1, [sp, #192]\n"
        "ldp %2, %3, [sp, #208]\n"
        : "=r"(x24), "=r"(x25), "=r"(x26), "=r"(x27)
    );

    asm volatile (
        "ldp %0, %1, [sp, #224]\n"
        "ldr %2, [sp, #240]\n"
        : "=r"(x28), "=r"(x29), "=r"(x30)
    );

    // Print the values of all GPRs
    printf("Register x0:  0x%016lx\n", x0);
    printf("Register x1:  0x%016lx\n", x1);
    printf("Register x2:  0x%016lx\n", x2);
    printf("Register x3:  0x%016lx\n", x3);
    printf("Register x4:  0x%016lx\n", x4);
    printf("Register x5:  0x%016lx\n", x5);
    printf("Register x6:  0x%016lx\n", x6);
    printf("Register x7:  0x%016lx\n", x7);
    printf("Register x8:  0x%016lx\n", x8);
    printf("Register x9:  0x%016lx\n", x9);
    printf("Register x10: 0x%016lx\n", x10);
    printf("Register x11: 0x%016lx\n", x11);
    printf("Register x12: 0x%016lx\n", x12);
    printf("Register x13: 0x%016lx\n", x13);
    printf("Register x14: 0x%016lx\n", x14);
    printf("Register x15: 0x%016lx\n", x15);
    printf("Register x16: 0x%016lx\n", x16);
    printf("Register x17: 0x%016lx\n", x17);
    printf("Register x18: 0x%016lx\n", x18);
    printf("Register x19: 0x%016lx\n", x19);
    printf("Register x20: 0x%016lx\n", x20);
    printf("Register x21: 0x%016lx\n", x21);
    printf("Register x22: 0x%016lx\n", x22);
    printf("Register x23: 0x%016lx\n", x23);
    printf("Register x24: 0x%016lx\n", x24);
    printf("Register x25: 0x%016lx\n", x25);
    printf("Register x26: 0x%016lx\n", x26);
    printf("Register x27: 0x%016lx\n", x27);
    printf("Register x28: 0x%016lx\n", x28);
    printf("Register x29 (FP): 0x%016lx\n", x29);  // x29 is the Frame Pointer (FP)
    printf("Register x30 (LR): 0x%016lx\n", x30);  // x30 is the Link Register (LR)

    // Restore all GPRs from the stack
    asm volatile (
        "ldp x0, x1, [sp, #0]\n"
        "ldp x2, x3, [sp, #16]\n"
        "ldp x4, x5, [sp, #32]\n"
        "ldp x6, x7, [sp, #48]\n"
        "ldp x8, x9, [sp, #64]\n"
        "ldp x10, x11, [sp, #80]\n"
        "ldp x12, x13, [sp, #96]\n"
        "ldp x14, x15, [sp, #112]\n"
        "ldp x16, x17, [sp, #128]\n"
        "ldp x18, x19, [sp, #144]\n"
        "ldp x20, x21, [sp, #160]\n"
        "ldp x22, x23, [sp, #176]\n"
        "ldp x24, x25, [sp, #192]\n"
        "ldp x26, x27, [sp, #208]\n"
        "ldp x28, x29, [sp, #224]\n"
        "ldr x30, [sp, #240]\n"
   
        "add sp, sp, #272\n"  // Restore the stack pointer back to its original position
    );
}


int main() {
    // Replace 'your_syscall_number' with the number assigned to your new syscall
    int result;
    //uint64_t current_ttbr = get_ttbr0_el1();
    //result = syscall(464);
    
    asm volatile (
     "mov x8, #463\t\n"
     "svc #0\n"  // Replace this with the correct opcode for cdummy x0
    );
    asm volatile (
    "mov %0, x0"   // Save the result from x0 into variable 'sum'
        : "=r" (result)        // Output operand
        :                   // No input operands
        : "x0"        // Clobbered registers
    );
    printf("syscall:cdummy (463): %d",result);

    asm volatile (
     "mov x8, #464\t\n"
     "svc #0\n"  // Replace this with the correct opcode for ccall x0
    );
    asm volatile (
     "mov x8, #465\t\n"
     "svc #0\n"  // Replace this with the correct opcd .code for cret x0
    );

    // //the callee's cret via cjmp should jump to here. 
    // printf("Caller-Data Owner: Successful return from 'addition' function that accessed operands via cross-domain caps.\nSum: %d\n",sum);
    
    // print_gpr_values();
    // asm volatile (
    // "mov x8, #464\n"
    // "svc #1\n"  // Replace this with the correct opcode for ccall x0
    // );
    // print_gpr_values();
    // //get_ttbr0_el1();

    // asm volatile (
    // "mov x8, #46\n"
    // "svc #2\n"  // Replace this with the correct opcode for ccall x0
    // );
    // print_gpr_values();
    // get_ttbr0_el1();

    return 0;
}
