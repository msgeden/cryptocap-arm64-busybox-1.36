#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "cc_header.h"

size_t total_size=0;

int main(int argc, char *argv[]) {

    int pipefd[2];
    
    if (argc > 1) {
        total_size = atol(argv[1])*MB_size;
        if (total_size == 0) {
            fprintf(stderr, "Invalid size provided.\n");
            return EXIT_FAILURE;
        }
    }
    
    pid_t pid;
    char buffer[BUFFER_SIZE];
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    uint64_t int_via_cap = 52;

    pid = fork();
    if (pid == 0) { // Child process

        close(pipefd[1]); 
        
        dup2(pipefd[0], STDIN_FILENO);  // Redirect pipe read end to stdin

        execl("./pipe-receiver-412.exe", "pipe-receiver-412.exe", NULL);
        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);
    
    } else { // Parent process

        close(pipefd[0]);
        
        //send the text
        char message_via_pipe[] = "Hello from sender via pipe-copy!";

        write(pipefd[1], message_via_pipe, sizeof(buffer));

        //send the cap for str
                                                                                                                                                                                                                                                                    
                                                                                                                                                                                                                                                                    
        //char message_via_cap[] = "Hello from sender via pipe-cap!";
 
        // Allocate a single contiguous block of memory for the entire data.
        char *message_via_cap = malloc(total_size+1);
        printf("Sender: malloc address:0x%lx\n", message_via_cap);

        if (!buffer) {
            perror("malloc");
            return EXIT_FAILURE;
        }
        
        // Seed the random number generator.
        srand((unsigned int) time(NULL));
        
        // Fill the entire buffer with random printable ASCII characters (from ' ' to '~').
        for (size_t i = 0; i < total_size; i++) {
            message_via_cap[i] = 'a' + (rand() % ('z' - 'a' + 1));
        }
        //printf("Sender: %s", message_via_cap);


        cc_dcap sent_cap_str=cc_create_signed_cap_on_creg0(message_via_cap, 0, strlen(message_via_cap) + 1, true);
        printf("Sender: ");
        cc_print_cap(sent_cap_str);

        //send the cap for int
        cc_dcap sent_cap_int=cc_create_signed_cap_on_creg0(&int_via_cap, 0, sizeof(uint64_t), true);
        printf("Sender: ");
        cc_print_cap(sent_cap_int);
        
        int_via_cap++;
        
        uint64_t int_value = cc_load_creg0_read_i64_data(sent_cap_int);
        printf("Sender: Integer received via intra-domain cap: %ld\n", int_value);

        //MAC to be signed by kernel within our custom write_cap function
        //write(pipefd[1], &sent_cap_str, sizeof(cc_dcap));
        //write(pipefd[1], &sent_cap_int, sizeof(cc_dcap));
        syscall(SYS_write_cap, pipefd[1], &sent_cap_str);
        syscall(SYS_write_cap, pipefd[1], &sent_cap_int);

        printf("Sender: close\n");


        close(pipefd[1]);
       
        printf("Sender: wait\n");

        wait(NULL); // Wait for the child process to finish
    }
    printf("Return\n");


    return 0;
}