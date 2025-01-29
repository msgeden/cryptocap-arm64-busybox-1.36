#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "cc-common.h"

// Dummy function for process 2
__attribute__((section(".fixed_section"))) 
int process2_dummy_func(void) {
    int result=2;
    printf("[Process 2]: Dummy function in process 2 called.\n");
    printf("[Process 2]: Calling process 3's dummy function...\n");
    result=syscall(SYS_pcall, getpid()+1, (void*)(entry_func));
    printf("[Process 2]: Process 3 returned with result:%d\n",result);
    syscall(SYS_pret, result);
    exit(0);
}

int main() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed for process 3");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process (Process 3)
        execl("./process3", "./process3", NULL);
        perror("Failed to exec process3");
        exit(EXIT_FAILURE);
    } else {
        
        pid_t cpid = getpid();
        printf("[Process 2]: PID:%d\n",cpid);
        wait_for_call_via_loop();
    }

    return 0;
}
