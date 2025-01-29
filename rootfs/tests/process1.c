#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "cc-common.h"

// Placeholder for calling process 2's function
__attribute__((section(".fixed_section"))) 
void call_cross_process_func(pid_t current_pid) {
    int result=1;
    printf("[Process 1]: Calling process 2's dummy function...\n");
    //result=syscall(SYS_pcall, current_pid+1, (uint64_t)&call_cross_process_func);
    printf("[Process 1]: Process 2 returned with result:%d\n",result);
}

int main() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed for process 2");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process (Process 2)
        execl("./process2", "./process2", NULL);
        perror("Failed to exec process2");
        exit(EXIT_FAILURE);
    } else {
        pid_t cpid = getpid();
        printf("[Process 1]: PID:%d\n",cpid);
        call_cross_process_func(cpid);
    }

    return 0;
}
