#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "cc_header.h"

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
    cc_memcpy_i8(recv_data, recv_cap_str, recv_cap_str.size);
    printf("Receiver: String received via pipe-cap: %s\n", recv_data);
    
    return 0;
}