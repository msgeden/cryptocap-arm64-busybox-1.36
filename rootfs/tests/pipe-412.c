#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "cc_header.h"
#define BUFFER_SIZE 100
char message_via_cap[] = "Hello from parent via cap!";

int main() {

    int pipefd1[2];
    int pipefd2[2];

    pid_t pid;
    char buffer[BUFFER_SIZE];
    
    if (pipe(pipefd1) == -1) {
        perror("pipe");
        return 1;
    }
    if (pipe(pipefd2) == -1) {
        perror("pipe");
        return 1;
    }
    uint64_t int_via_cap = 52;

    pid = fork();
    if (pid == 0) { // Child process
        char recv_data[BUFFER_SIZE];

        close(pipefd1[1]); 
        close(pipefd2[1]); 
        
        //receive the actual data
        read(pipefd1[0], recv_data, sizeof(recv_data));
        printf("Child: String received via pipe-copy: %s\n", recv_data);
        
        //receive the cap 
        cc_dcap recv_cap;
        read(pipefd2[0], &recv_cap, sizeof(cc_dcap));
        cc_print_cap(recv_cap);
        
        cc_dcap recv_cap_local;
        read(pipefd2[0], &recv_cap_local, sizeof(cc_dcap));
        cc_print_cap(recv_cap_local);

        uint64_t recv_int_local = cc_load_creg0_read_i64_data(recv_cap_local);
        printf("Child: Integer received via pipe-cap-local: %ld\n", recv_int_local);

        char msg_char = cc_load_creg0_read_i8_data(recv_cap);
        printf("Child:  Char received via pipe-cap: %c\n", msg_char);

        //IMPORTANT: there has to be a copy for call by reference functions unless all pointers are capabilities
        cc_memcpy_i8_asm(recv_data, recv_cap, recv_cap.size);
        printf("Child: String received via pipe-cap: %s\n", recv_data);

        close(pipefd1[0]);
        close(pipefd2[0]);
    } else { // Parent process

        close(pipefd1[0]);
        close(pipefd2[0]);

        //send the text
        char message_via_pipe[] = "Hello from parent via pipe!";
        write(pipefd1[1], message_via_pipe, sizeof(buffer) + 1);

        //send the cap
        //String here is not on the stack
        cc_dcap sent_cap=cc_create_signed_cap_on_creg0(message_via_cap, 0, strlen(message_via_cap) + 1, false);
        cc_print_cap(sent_cap);

        write(pipefd2[1], &sent_cap, sizeof(cc_dcap));

        int_via_cap++;
        cc_dcap sent_cap_local=cc_create_signed_cap_on_creg0(&int_via_cap, 0, sizeof(uint64_t), false);
        cc_print_cap(sent_cap_local);
        
        //MAC to be signed by kernel within our custom write_cap function
        write(pipefd2[1], &sent_cap_local, sizeof(cc_dcap));

        close(pipefd1[1]);
        close(pipefd2[1]);

        wait(NULL); // Wait for the child process to finish
    }

    return 0;
}