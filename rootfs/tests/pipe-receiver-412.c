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


    //receive the actual data
    char recv_data[BUFFER_SIZE];
    read(STDIN_FILENO, recv_data, sizeof(recv_data));
    printf("Receiver: String received via pipe-copy: %s\n", recv_data);
        
    //receive the str cap 
    cc_dcap recv_cap_str;
    syscall(SYS_read_cap, STDIN_FILENO, &recv_cap_str);
    printf("Receiver: "); cc_print_cap(recv_cap_str);

    //receive the int cap 
    cc_dcap recv_cap_int;
    syscall(SYS_read_cap, STDIN_FILENO, &recv_cap_int);
    printf("Receiver: "); cc_print_cap(recv_cap_int);

    uint64_t recv_int_val = cc_load_creg0_read_i64_data(recv_cap_int);
    printf("Receiver: Integer received via cross-domain cap: %ld\n", recv_int_val);

    recv_int_val = cc_read_i64_via_creg0();
    printf("Receiver: Integer received via cross-domain cap: %ld\n", recv_int_val);

    cc_load_creg0_write_i64_data(recv_cap_int,54);
    
    recv_int_val = cc_load_creg0_read_i64_data(recv_cap_int);
    printf("Receiver: Integer modified via cross-domain cap: %ld\n", recv_int_val);

    cc_store_cap_from_creg0(&recv_cap_int);
    cc_load_ver_cap_to_creg0(&recv_cap_int);

    //IMPORTANT: there has to be a copy for call by reference functions unless all pointers are capabilities
    char* recv_str=malloc(recv_cap_str.size+1);
    printf("Receiver: malloc address:0x%lx\n", recv_str);
    cc_memcpy_i8_dbg(recv_str, recv_cap_str, recv_cap_str.size);
   
   
    // printf("Receiver: String received via pipe-cap: %s\n", recv_str);

    // //receive the str cap 
    // cc_dcap recv_cap_str;
    // read(STDIN_FILENO, &recv_cap_str, sizeof(cc_dcap));
    // //read_cap(STDIN_FILENO, &recv_cap_str);
    // printf("Receiver: "); cc_print_cap(recv_cap_str);

    // //IMPORTANT: there has to be a copy for call by reference functions unless all pointers are capabilities
    // char* recv_str=malloc(recv_cap_str.size);
    // printf("Receiver: allocated heap (malloc) base:0x%p\n", recv_str);
    // start_t = clock();
    // cc_memcpy_i8(recv_str, recv_cap_str, recv_cap_str.size);
    // end_t = clock();
    // total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;    
    // printf("Receiver: memcpy is completed for %.2f of data (MB) in %.2f seconds\n", (double)recv_cap_str.size/(1024*1024), total_t);

    // // --- Resume sender process---
    // cc_resume_process(getppid());

    // // Free the allocated memory
    // free(recv_str); 

    return 0;
}