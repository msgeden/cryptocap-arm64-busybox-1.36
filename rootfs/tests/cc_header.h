#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define SYS_dgrant 292  // Adjust syscall numbers as needed
#define SYS_read_cap 294  // Adjust syscall numbers as needed
#define SYS_write_cap 295  // Adjust syscall numbers as needed

#define BUFFER_SIZE 100
#define PIPE_SIZE 1024

size_t KB_size = 1024;

typedef enum capPermFlags {
    READ = 1,
    WRITE = 2,
    EXEC = 4,
    TRANS = 8,
} capPermFlagsType;

typedef struct cc_dcap {
     uint64_t perms_base;
     uint32_t offset;
     uint32_t size;
     uint64_t PT;
     uint64_t MAC;
} cc_dcap;

typedef struct cc_dcapcl {
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

} cc_dcapcl;

void cc_resume_process(pid_t pid) {
    kill(pid, SIGCONT);
    return;
}

void cc_suspend_process(pid_t pid) {
    kill(pid, SIGSTOP);
    return;
}
    
// A simple wrapper for the custom write_cap syscall.
ssize_t write_cap(int fd, const void *buf) {
    return syscall(SYS_write_cap, fd, buf);
}

// A simple wrapper for the custom read_cap syscall.
ssize_t read_cap(int fd, const void *buf) {
    return syscall(SYS_read_cap, fd, buf);
}

static void cc_print_cap(cc_dcap cap) {
    uint64_t perms=(cap.perms_base&0xFFFF000000000000)>>48;
    uint64_t base=cap.perms_base&0x0000FFFFFFFFFFFF;
    printf("cap.perms=0x%lx, .base=0x%lx, .offset=%d, .size=%d, .PT=0x%lx, .MAC=0x%lx\n", perms, base, cap.offset, cap.size, cap.PT, cap.MAC);
}

static uint64_t cc_get_PT() {
    uint64_t ttbr0_el1=0xDEADBEEF;
    asm volatile(".word 0x03300000" : "=r"(ttbr0_el1));  //readttbr x0
    return ttbr0_el1;
}

static void cc_load_ver_cap_to_creg0(cc_dcap* cap){
    asm volatile (
         "mov x9, %0\n\t"
         ".word 0x02000009\n\t"      // ldc cr0, [x9]
         :   
         : "r" (cap)
         : "x9" // clobber list 
    );
}

static void cc_store_cap_from_creg0(cc_dcap* cap){

    asm volatile (
         "mov x9, %0\n\t"
         ".word 0x02100009\n\t"      // stc cr0, [x9]
         :   
         : "r" (cap)
         : "x9" // clobber list 
    );
}

static cc_dcap cc_create_struct(void* base, size_t size, bool write_flag, uint64_t PT){
    cc_dcap cap;
    uint64_t perms=(write_flag)?WRITE:READ;
    perms=(perms<<48);
    cap.perms_base = perms | (uint64_t)base;
    cap.offset = 0;
    cap.size = size;
    cap.PT = PT;
    cap.MAC= 0x0;
    return cap;
}    

static cc_dcap cc_create_signed_cap_on_creg0(void* base, size_t offset, size_t size, bool write_flag){
    cc_dcap cap;
    uint64_t perms=(write_flag)?WRITE+READ:READ;
    perms=(perms<<48);
    uint64_t perms_base = perms | (uint64_t)base;
    uint64_t offset_size = ((uint64_t)size << 32) | (uint64_t)offset;
    asm volatile (
        "mov x9, %0\n\t" // mov perms_base to x9
        "mov x10, %1\n\t" // mov offset+size to x10
        ".word 0x0340012a\n\t"  //ccreate cr0, x9, x10  
        : 
        : "r" (perms_base), "r" (offset_size)
        : "x9", "x10"
    );
    cc_store_cap_from_creg0(&cap);
    return cap;
}    

static cc_dcap cc_setbase_resign_on_creg0(void* new_base, cc_dcap original_cap){
    cc_load_ver_cap_to_creg0(&original_cap);
    cc_dcap new_cap;
    asm volatile (
        "mov x9, %0\n\t" // mov new_base to x9
        ".word 0x02500009\n\t"  //csetbase cr0, cr0, x9  
        : 
        : "r" (new_base)
        : "x9", "x10"
    );
    cc_store_cap_from_creg0(&new_cap);
    return new_cap;
}    


static cc_dcap cc_setperms_resign_on_creg0(uint32_t perms, cc_dcap original_cap){
    cc_load_ver_cap_to_creg0(&original_cap);
    cc_dcap new_cap;
    asm volatile (
        "mov x9, %0\n\t" // mov new_base to x9
        ".word 0x02700009\n\t"  //setsize cr0, cr0, x9  
        : 
        : "r" (perms)
        : "x9", "x10"
    );
    cc_store_cap_from_creg0(&new_cap);
    return new_cap;
}    

static cc_dcap cc_setsize_resign_on_creg0(uint32_t size, cc_dcap original_cap){
    cc_load_ver_cap_to_creg0(&original_cap);
    cc_dcap new_cap;
    asm volatile (
        "mov x9, %0\n\t" // mov new_base to x9
        ".word 0x02600009\n\t"  //setsize cr0, cr0, x9  
        : 
        : "r" (size)
        : "x9", "x10"
    );
    cc_store_cap_from_creg0(&new_cap);
    return new_cap;
}    

static void cc_inc_cap_offset(cc_dcap* cap, uint32_t leap){
    cap->offset+=leap;
}

static inline uint8_t cc_read_i8_via_creg0(){
        uint8_t data=0;
        asm volatile (
            ".word 0x02200d20\n\t"  //cldg8 x9, [cr0] (operand_1) 
            //".word 0x02200c00\n\t"  //cldg8 x0, [cr0] (operand_1) 
            "and %0, x9, #0xFF\n\t" // Extract the least significant byte from x9
            : "=r" (data)  
            :
            : "x9", "memory"
        );
        return data;
}

static inline void cc_write_i8_via_creg0(uint8_t data){
    asm volatile (
        "and x9, %0, #0xFF\n\t" // Extract the least significant byte from x9
        ".word 0x02300d20\n\t"     //cstg8 x9, [cr0] (operand_1) 
        :   
        : "r" (data)
        : "x9"
    );
    return;
}

static uint8_t cc_load_creg0_read_i8_data(cc_dcap cap){
    uint8_t data=0;
    cc_load_ver_cap_to_creg0(&cap);
    asm volatile (
        ".word 0x02200d20\n\t"  //cldg8 x9, [cr0] (operand_1) 
        "and %0, x9, #0xFF\n\t" // Extract the least significant byte from x9
        //"mov %0, x9\n\t"        // move x9 the register of data variable
        : "=r" (data)  
        :
        : "x9", "memory"
    );
    return data;
}

static void cc_load_creg0_write_i8_data(cc_dcap cap, uint8_t data){
    cc_load_ver_cap_to_creg0(&cap);
    asm volatile (
        "and x9, %0, #0xFF\n\t" // Extract the least significant byte to x9
        ".word 0x02300d20\n\t"     //cstg8 x9, [cr0] (operand_1) 
        :   
        : "r" (data)
        : "x9"
    );
    return;
}

static inline uint16_t cc_read_i16_via_creg0(){
    uint16_t data;
    asm volatile (
        ".word 0x02200920\n\t"  //cldg16 x9, [cr0] (operand_1) 
        "and %0, x9, #0xFFFF\n\t" // Extract the least significant byte from x9
        : "=r" (data)  
        :
        : "x9", "memory"
    );
    return data;
}

static inline void cc_write_i16_data_via_creg0(uint16_t data){

    asm volatile (
        "and x9, %0, #0xFFFF\n\t" // Extract the least significant byte to x9
        ".word 0x02300920\n\t"     //cstg16 x9, [cr0] (operand_1) 
        :   
        : "r" (data)
        : "x9"
    );
    return;
}

static uint16_t cc_load_creg0_read_i16_data(cc_dcap cap){
    uint16_t data;
    cc_load_ver_cap_to_creg0(&cap);
    asm volatile (
        ".word 0x02200920\n\t"  //cldg16 x9, [cr0] (operand_1) 
        "and %0, x9, #0xFFFF\n\t" // Extract the least significant byte from x9
        : "=r" (data)  
        :
        : "x9", "memory"
    );
    return data;
}

static void cc_load_creg0_write_i16_data(cc_dcap cap, uint16_t data){
    cc_load_ver_cap_to_creg0(&cap);
    asm volatile (
        "and x9, %0, #0xFFFF\n\t" // Extract the least significant byte to x9
        ".word 0x02300920\n\t"     //cstg16 x9, [cr0] (operand_1) 
        :   
        : "r" (data)
        : "x9"
    );
    return;
}

static inline uint32_t cc_read_i32_data_via_creg0(){
    uint32_t data;
    asm volatile (
        ".word 0x02200520\n\t"  //cldg x9, [cr0] (operand_1) 
        "and %0, x9, #0xFFFFFFFF\n\t" // Extract the least significant byte from x9
        : "=r" (data)  
        :
        : "x9", "memory"
    );
    return data;
}

static inline void cc_write_i32_data_via_creg0(uint32_t data){
    asm volatile (
        "and x9, %0, #0xFFFFFFFF\n\t" // Extract the least significant byte to x9
        ".word 0x02300520\n\t"     //cstg x9, [cr0] (operand_1) 
        :   
        : "r" (data)
        : "x9"
    );
    return;
}

static uint32_t cc_load_creg0_read_i32_data(cc_dcap cap){
    uint32_t data;
    cc_load_ver_cap_to_creg0(&cap);
    asm volatile (
        ".word 0x02200520\n\t"  //cldg x9, [cr0] (operand_1) 
        "and %0, x9, #0xFFFFFFFF\n\t" // Extract the least significant byte from x9
        : "=r" (data)  
        :
        : "x9", "memory"
    );
    return data;
}

static void cc_load_creg0_write_i32_data(cc_dcap cap, uint32_t data){
    cc_load_ver_cap_to_creg0(&cap);
    asm volatile (
        "and x9, %0, #0xFFFFFFFF\n\t" // Extract the least significant byte to x9
        ".word 0x02300520\n\t"     //cstg x9, [cr0] (operand_1) 
        :   
        : "r" (data)
        : "x9"
    );
    return;
}


static inline uint64_t cc_read_i64_via_creg0(){
    uint64_t data=0;
    asm volatile (
        ".word 0x02200120\n\t"  //cldg x9, [cr0] (operand_1) 
        "mov %0, x9\n\t"        // move x9 the register of data variable
        : "=r" (data)  
        :
        : "x9", "memory"
    );
    return data;
}

static inline void cc_write_i64_via_creg0(uint64_t data){
    asm volatile (
        "mov x9, %0\n\t"          // move data variable to x9
        ".word 0x02300120\n\t"     //cstg x9, [cr0] (operand_1) 
        :   
        : "r" (data)
        : "x9"
    );
    return;
}

static uint64_t cc_load_creg0_read_i64_data(cc_dcap cap){
    uint64_t data;
    cc_load_ver_cap_to_creg0(&cap);
    asm volatile (
        ".word 0x02200120\n\t"  //cldg x9, [cr0] (operand_1) 
        "mov %0, x9\n\t"        // move x9 the register of data variable
        : "=r" (data)  
        :
        : "x9", "memory"
    );
    return data;
}

static void cc_load_creg0_write_i64_data(cc_dcap cap, uint64_t data){
    cc_load_ver_cap_to_creg0(&cap);
    asm volatile (
        "mov x9, %0\n\t"          // move data variable to x9
        ".word 0x02300120\n\t"     //cstg x9, [cr0] (operand_1) 
        :   
        : "r" (data)
        : "x9"
    );
    return;
}

static uint8_t* cc_memcpy_i8_asm_new(void* dst, cc_dcap src, size_t count) {
    cc_dcap cap;
    uint8_t* dest = (uint8_t*)dst;
    if (count == 0) return dst;
    
    cc_load_ver_cap_to_creg0(&src);

    asm volatile(
        "mov x10, %[dst_ptr]\n\t"    // Load dst into x10 (using operand name)
        "mov x11, %[cnt]\n\t"        // Load count into x11
        "mov x12, #0\n\t"            // Initialize iterator
        "1:\n\t"
        ".word 0x02200d20\n\t"       // cldg8 x9, [cr0] 
        ".word 0x03c00001\n\t"       // cincoffset cr0, #1 
        "strb w9, [x10]\n\t"         // Store byte to [x10]
        "add x10, x10, #1\n\t"       // Increment dst
        "add x12, x12, #1\n\t"       // Increment iterator
        "cmp x12, x11\n\t"
        "b.lt 1b\n\t"
        : 
        : [dst_ptr] "r" (dst), [cnt] "r" (count)          // %1: count is input
        : "x9", "x10", "x11", "x12", "cc", "memory"
    );
    return dst;

}
static uint8_t* cc_memcpy_i8_asm(void* dst, cc_dcap src, size_t count) {
    cc_dcap cap;
    uint8_t* dest = (uint8_t*)dst;
    if (count == 0) return dst;
    
    // Handle first byte outside the loop
    dest[0] = cc_load_creg0_read_i8_data(src);

    if (count <= 1) return dst;
    
    asm volatile(
        "mov x10, %[dst_ptr]\n\t"    // Load dst into x10 (using operand name)
        "mov x11, %[cnt]\n\t"        // Load count into x11
        "add x10, x10, #1\n\t"       // Increment dst address
        "mov x12, #1\n\t"            // Initialize iterator
        "1:\n\t"
        ".word 0x02200d20\n\t"       // cldg8 x9, [cr0] 
        ".word 0x03c00001\n\t"       // cincoffset cr0, #1 
        "strb w9, [x10]\n\t"         // Store byte to [x10]
        "add x10, x10, #1\n\t"       // Increment dst
        "add x12, x12, #1\n\t"       // Increment iterator
        "cmp x12, x11\n\t"
        "b.lt 1b\n\t"
        : 
        : [dst_ptr] "r" (dst), [cnt] "r" (count)          // %1: count is input
        : "x9", "x10", "x11", "x12", "cc", "memory"
    );
    return dst;
}

static uint8_t* ncc_memcpy_i8_asm(void* dst, void* src, size_t count) {
    uint8_t* dest = (uint8_t*)dst;
    uint8_t* source = (uint8_t*)src;

    if (count == 0) return dst;
    
    // Handle first byte outside the loop
    dest[0] = source[0];

    if (count <= 1) return dst;

    asm volatile(
        "mov x10, %[dst_ptr]\n\t"    // Load dst into x10
        "mov x11, %[src_ptr]\n\t"    // Load src into x11
        "mov x12, %[cnt]\n\t"        // Load count into x12
        "add x10, x10, #1\n\t"       // Increment dst address
        "add x11, x11, #1\n\t"       // Increment src address
        "mov x13, #1\n\t"            // Initialize iterator
        "1:\n\t"
        "ldrb w9, [x11]\n\t"         // Load byte from [src] into w9
        "strb w9, [x10]\n\t"         // Store byte to [dst]
        "add x10, x10, #1\n\t"       // Increment dst pointer
        "add x11, x11, #1\n\t"       // Increment src pointer
        "add x13, x13, #1\n\t"       // Increment iterator
        "cmp x13, x12\n\t"
        "b.lt 1b\n\t"                // Loop if i < count
        : 
        : [dst_ptr] "r" (dst), [src_ptr] "r" (src), [cnt] "r" (count)
        : "x9", "x10", "x11", "x12", "x13", "cc", "memory"
    );

    return dst;
}

static uint8_t* cc_memcpy_i8_dbg(char* dst, cc_dcap src, size_t count) {
    uint64_t src_addr;
    cc_load_ver_cap_to_creg0(&src);
    for (int i = 0; i < count; i++) {
        dst[i] = cc_read_i8_via_creg0();
        if (i % 100 == 0) {
            printf("memcpy_i8: address copy dst:%p\n", &dst[i]);
            printf("memcpy_i8: address copy src:%p\n", (void*)(src_addr+i));
        }
        asm volatile (
            ".word 0x03c00001\n\t"       // cincoffset cr0, #1 
        );
    }
    return dst;
}


static uint8_t* cc_memcpy_i8(void* dst, cc_dcap src, size_t count) {
    uint8_t* dest = (uint8_t*)dst;
    dest[0] = cc_load_creg0_read_i8_data(src);
    for (int i = 1; i < count; i++) {
        asm volatile (
            //".word 0x02f00c09\n\t"      // cmanip cr0[1]/offset, R, x9
            //"add x9, x9, #1\n\t"
            //".word 0x02f00809\n\t"      // cmanip cr0[1]/offset, W, x9
            ".word 0x03c00001\n\t"         //"cincoffset cr0, #1

        );
        dest[i] = cc_read_i8_via_creg0();
    }
    return dst;
}

static uint8_t* cc_memcpy_unoptimised(void* dst, cc_dcap src, size_t count) {
    uint8_t* dest = (uint8_t*)dst;
    dest[0] = cc_load_creg0_read_i8_data(src);
    for (int i = 1; i < count; i++) {
        cc_inc_cap_offset(&src,1);
        dest[i] = cc_load_creg0_read_i8_data(src);
    }
    return dst;
}

static int cc_dcall(){ 
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

static int cc_dret(){ 
   asm volatile(
        ".word 0x3e00000\n\t"      // dret
        "ret\n\t"
     );
}

void cc_wait_for_call_via_loop(void){
    while(1) {
        sleep(1);  // Wait for calls
    }
}