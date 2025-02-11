#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>


#include "cc_header.h"
size_t total_size=0;
clock_t start_t, end_t;
double total_t;
double sender_t;

int main() {


    clock_t start_t, end_t;
    double total_t;
    
    cc_dcap recv_cap_str;
    read(STDIN_FILENO, &recv_cap_str, sizeof(cc_dcap));
    printf("Receiver:\t"); cc_print_cap(recv_cap_str);

    start_t = clock();
    char* recv_copy=malloc(recv_cap_str.size);
    cc_memcpy_i8_asm(recv_copy, recv_cap_str, recv_cap_str.size);
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;  
    //printf("Receiver: String received via pipe-cap: %s\n", copied_str);
    printf("Receiver:\tcross copy via (cap) CLDG instruction (single-byte):\t%.2f of data (MB) in\t%.2f seconds\n", (double)recv_cap_str.size/MB_size, total_t);
    free(recv_copy);
  
    // // --- Resume sender process---
    printf("Receiver:\tResuming the sender process via SIGCONT\n");
    cc_resume_process(getppid());

    return 0;
}