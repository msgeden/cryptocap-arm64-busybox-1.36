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

size_t total_size_t=0;
clock_t start_t, end_t;
double total_t;
double sender_t;

int main() {

    //receive the actual data
    // char recv_data[BUFFER_SIZE];
    // read(STDIN_FILENO, recv_data, sizeof(recv_data));
    // printf("Receiver: String received via pipe-copy: %s\n", recv_data);


    // //receive the str cap 
    cc_dcap recv_cap_str;
    //syscall(SYS_read_cap, STDIN_FILENO, &recv_cap_str);
    read(STDIN_FILENO, &recv_cap_str, sizeof(cc_dcap));

    printf("Receiver: "); cc_print_cap(recv_cap_str);

    start_t = clock();
    char* cross_copy=malloc(recv_cap_str.size);

    cc_memcpy_i8_asm_new(cross_copy, recv_cap_str, recv_cap_str.size);
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;  
    //printf("Receiver: String received via pipe-cap: %s\n", copied_str);
    printf("Receiver: cross-domain memcpy via cldg instruction (at byte-level) is completed for %.2f of data (MB) in %.2f seconds\n", (double)recv_cap_str.size/(1024*1024), total_t);
    free(cross_copy);
    cc_resume_process(getppid());

    return 0;
}