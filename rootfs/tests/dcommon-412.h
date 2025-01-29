#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define SYS_dgrant 292  // Adjust syscall numbers as needed

#define READ_TTBR(reg) \
    asm volatile (".word %0\n\t" : : "i"(0x03300000 | ((reg) & 0x1F)) : "memory")


typedef struct dcap {
     uint64_t perms_base;
     uint32_t offset;
     uint32_t size;
     uint64_t PT;
     uint64_t MAC;
} dcap;

typedef struct dcapcl {
     uint64_t PC;
     uint64_t SP_EL0;
     uint64_t TTBR0_EL1;
     uint64_t TTBR1_EL1;
     uint64_t TASK_STRUCT;
     uint64_t PSTATE;
     uint64_t TPIDR_EL0;
     uint64_t TPIDRRO_EL0;
     uint64_t FIELD_9;
     uint64_t FIELD_10;
     uint64_t FIELD_11;
     uint64_t FIELD_12;

} dcapcl;



void wait_for_call_via_loop(void){
    while(1) {
        sleep(1);  // Wait for calls
    }
}

uint64_t getPTBase() {
    uint64_t value=0xDEADBEEF;
    asm volatile(".word 0x03300000" : "=r"(value));  //readttbr x0
    printf("get_ttbr0_el1(): 0x%lx\n", value);
    return value;
}

__attribute__((naked))
static int dcall(){ 
    asm volatile (
        // Reserve 256 bytes on the stack
        "sub     sp, sp, #256\n\t"
        "str	x30, [sp, #8]\n\t"
        "str	x29, [sp, #16]\n\t"
        "str	x28, [sp, #24]\n\t"
        "str	x27, [sp, #32]\n\t"
        "str	x26, [sp, #40]\n\t"
        "str	x25, [sp, #48]\n\t"
        "str	x24, [sp, #56]\n\t"
        "str	x23, [sp, #64]\n\t"
        "str	x22, [sp, #72]\n\t"
        "str	x21, [sp, #80]\n\t"
        "str	x20, [sp, #88]\n\t"
        "str	x19, [sp, #96]\n\t"
        "str	x18, [sp, #104]\n\t"
        "str	x17, [sp, #112]\n\t"
        "str	x16, [sp, #120]\n\t"
        "str	x15, [sp, #128]\n\t"
        "str	x14, [sp, #136]\n\t"
        "str	x13, [sp, #144]\n\t"
        "str	x12, [sp, #152]\n\t"
        "str	x11, [sp, #160]\n\t"    
        "str	x10, [sp, #168]\n\t"
        "str	x9, [sp, #176]\n\t"
        // "str	x8, [sp, #184]\n\t"
        // "str	x7, [sp, #192]\n\t"
        // "str	x6, [sp, #200]\n\t"
        // "str	x5, [sp, #208]\n\t"
        // "str	x4, [sp, #216]\n\t"
        // "str	x3, [sp, #224]\n\t"
        // "str	x2, [sp, #232]\n\t"
        // "str	x1, [sp, #240]\n\t"
        // "str	x0, [sp, #248]\n\t"
    );
    asm volatile(
        ".word 0x3d00000\n\t"      // dcall
    );
    asm volatile (
        // "ldr x0, [sp, #248]\n\t"
        // "ldr x1, [sp, #240]\n\t"
        // "ldr x2, [sp, #232]\n\t"
        // "ldr x3, [sp, #224]\n\t"
        // "ldr x4, [sp, #216]\n\t"
        // "ldr x5, [sp, #208]\n\t"
        // "ldr x6, [sp, #200]\n\t"
        // "ldr x7, [sp, #192]\n\t"
        // "ldr x8, [sp, #184]\n\t"
        "ldr x9, [sp, #176]\n\t"
        "ldr x10, [sp, #168]\n\t"
        "ldr x11, [sp, #160]\n\t"
        "ldr x12, [sp, #152]\n\t"
        "ldr x13, [sp, #144]\n\t"
        "ldr x14, [sp, #136]\n\t"
        "ldr x15, [sp, #128]\n\t"
        "ldr x16, [sp, #120]\n\t"
        "ldr x17, [sp, #112]\n\t"
        "ldr x18, [sp, #104]\n\t"
        "ldr x19, [sp, #96]\n\t"
        "ldr x20, [sp, #88]\n\t"
        "ldr x21, [sp, #80]\n\t"
        "ldr x22, [sp, #72]\n\t"
        "ldr x23, [sp, #64]\n\t"
        "ldr x24, [sp, #56]\n\t"
        "ldr x25, [sp, #48]\n\t"
        "ldr x26, [sp, #40]\n\t"
        "ldr x27, [sp, #32]\n\t"
        "ldr x28, [sp, #24]\n\t"
        "ldr x29, [sp, #16]\n\t"
        "ldr x30, [sp, #8]\n\t"
        "add     sp, sp, #256\n\t"
        "ret\n\t"

    );
}

__attribute__((naked))
static int dret(){ 
   asm volatile(
        ".word 0x3e00000\n\t"      // dret
        "ret\n\t"
     );
}
