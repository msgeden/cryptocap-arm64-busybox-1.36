#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "cc_header.h"
#define BUFFER_SIZE 100
char message_via_cap[] = "Hello from parent via cap!";
size_t total_size=0;

int main(int argc, char *argv[])  {

    if (argc > 1) {
        total_size = atol(argv[1])*KB_size;
        if (total_size == 0) {
            fprintf(stderr, "Invalid size provided.\n");
            return EXIT_FAILURE;
        }
    }
    
    int pipefd1[2];
    int pipefd2[2];


    pid_t pid;
    
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
       
        close(pipefd1[1]); 
        close(pipefd2[1]); 

        clock_t start_t_c, end_t_c;
        double total_t_c;
        
        // char* recv_str=malloc(total_size*sizeof(char));

        // //receive the actual data
        // start_t_c = clock();
        // read(pipefd1[0], recv_str, total_size);
        // end_t_c = clock();
        // total_t_c = (double)(end_t_c - start_t_c) / CLOCKS_PER_SEC;
        // printf("\nChild: Total time (s) of data read (%d KB):\t %f\n", (total_size/KB_size), total_t_c);
        // printf("Child: String received via piped-copy:\n");
        
        //receive the cap for local integer
        cc_dcap recv_cap_stack_int;
        read(pipefd2[0], &recv_cap_stack_int, sizeof(cc_dcap));
        printf("Child: "); cc_print_cap(recv_cap_stack_int);

        uint64_t recv_stack_int = cc_load_CR0_read_i64_data(recv_cap_stack_int);
        printf("Child: Integer received via piped-cap: %ld\n", recv_stack_int);

        //receive the cap for heap string
        cc_dcap recv_cap_heap_str;
        read(pipefd2[0], &recv_cap_heap_str, sizeof(cc_dcap));
        printf("Child: "); cc_print_cap(recv_cap_heap_str);
        
        char* recv_heap_str = malloc(recv_cap_heap_str.size*sizeof(char));
        //IMPORTANT: there has to be a copy for call by reference functions unless all pointers are capabilities
        start_t_c = clock();
        cc_memcpy_i8(recv_heap_str, recv_cap_heap_str, recv_cap_heap_str.size);
        end_t_c = clock();
        total_t_c = (double)(end_t_c - start_t_c) / CLOCKS_PER_SEC;
        printf("\nChild: Total time (s) of memcpy (%d KB):\t %f\n", (total_size/KB_size), total_t_c);
        printf("Child: memcpy_i8 is completed\n");
        //printf("Child: String received via pipe-cap: %s\n", recv_heap_str);

        close(pipefd1[0]);
        close(pipefd2[0]);

    } else { // Parent process

        close(pipefd1[0]);
        close(pipefd2[0]);
        
        clock_t start_t_p, end_t_p;
        double total_t_p;
        
        char* str=malloc(total_size*sizeof(char));
        for (size_t i = 0; i < total_size-1; i++) {
            //message_via_cap[i] = 'a' + (rand() % ('z' - 'a' + 1));
            str[i] = 'a' + (i % 27);
        }
        str[total_size-1]=0;
        //send the text

        // start_t_p = clock();
        // write(pipefd1[1], str, total_size);
        // end_t_p = clock();
        // total_t_p = (double)(end_t_p - start_t_p) / CLOCKS_PER_SEC;
        // printf("Parent: Total time (s) of data write (%d KB):\t %f\n", (total_size/KB_size), total_t_p);
        
        int_via_cap++;
        //send the cap for int
        cc_dcap sent_cap_int=cc_create_signed_cap_on_CR0(&int_via_cap, 0, sizeof(uint64_t), false);
        printf("Parent: "); cc_print_cap(sent_cap_int);
        write(pipefd2[1], &sent_cap_int, sizeof(cc_dcap));

        uint64_t local_stack_int = cc_load_CR0_read_i64_data(sent_cap_int);
        printf("Parent: Integer received via local-cap: %ld\n", local_stack_int);

        //send the cap for str
        cc_dcap sent_cap_str=cc_create_signed_cap_on_CR0(str, 0, total_size, false);
        printf("Parent: "); cc_print_cap(sent_cap_str);
       
        start_t_p = clock();
        write(pipefd2[1], &sent_cap_str, sizeof(cc_dcap));
        end_t_p = clock();
        total_t_p = (double)(end_t_p - start_t_p) / CLOCKS_PER_SEC;
        printf("Parent: Total time (s) of cap write:\t %f\n", total_t_p);
        

        close(pipefd1[1]);
        close(pipefd2[1]);

        wait(NULL); // Wait for the child process to finish
    }

    return 0;
}