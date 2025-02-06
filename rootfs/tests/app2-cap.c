#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <ctype.h>
#include "cc_header.h"

int main(void) {
    int size;
    cc_dcap recv_cap_str;
    read(STDIN_FILENO, &recv_cap_str, sizeof(cc_dcap));
    size=recv_cap_str.size;
    cc_print_cap(recv_cap_str);
    //scanf("%d", &size); 
    char* recv_buffer=(char*)malloc(recv_cap_str.size*sizeof(char));
    //printf("Hello null: %s\n", recv_buffer);

    cc_memcpy_i8(recv_buffer, recv_cap_str, recv_cap_str.size);
    printf("Hello lowercase: %s\n", recv_buffer);
    //scanf("%s", recv_buffer); 
    for (int i = 0; i<size-1; i++) {
        recv_buffer[i] = toupper(recv_buffer[i]); // Convert to uppercase
    }
    printf("Hello uppercase: %s\n", recv_buffer);
    printf("\n");
    free(recv_buffer);
    return 0;
}

