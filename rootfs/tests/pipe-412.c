#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "cc_header.h"
#define BUFFER_SIZE 100
char message_via_cap[] = "Hello from parent via cap!";
//uint64_t message_int=42;
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
    pid = fork();
    if (pid == 0) { // Child process
        char recv_data[BUFFER_SIZE];

        close(pipefd1[1]); 
        close(pipefd2[1]); 
        
        //receive the actual data
        read(pipefd1[0], recv_data, sizeof(recv_data));
        printf("String received via pipe-copy: %s\n", recv_data);
        
        //receive the cap 
        cc_dcap recv_cap;
        read(pipefd2[0], &recv_cap, sizeof(cc_dcap));
        
        char msg_char = cc_read_i8_data_via_creg0(recv_cap);
        printf("Char received via pipe-cap: %c\n", msg_char);

        //IMPORTANT: there has to be a copy for call by reference functions unless all pointers are capabilities
        cc_memcpy(recv_data, recv_cap, recv_cap.size);
        printf("String received via pipe-cap: %s\n", recv_data);
        
        close(pipefd1[0]);
        close(pipefd2[0]);
    } else { // Parent process

        close(pipefd1[0]);
        close(pipefd2[0]);

        //send the text
        char message_via_pipe[] = "Hello from parent via pipe!";
        write(pipefd1[1], message_via_pipe, sizeof(buffer) + 1);

        //send the cap
        char message_via_cap[] = "Hello from parent via cap!";
        cc_dcap sent_cap=cc_create_cap_struct(message_via_cap, strlen(message_via_cap) + 1, false, cc_get_PT());

        //MAC to be signed by kernel within our custom write_cap function
        write(pipefd2[1], &sent_cap, sizeof(cc_dcap));

        close(pipefd1[1]);
        close(pipefd2[1]);

        wait(NULL); // Wait for the child process to finish
    }

    return 0;
}