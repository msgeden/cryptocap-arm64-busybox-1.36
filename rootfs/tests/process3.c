#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "cc-common.h"

// Dummy function for process 3
__attribute__((section(".fixed_section"))) 
void process3_dummy_func() {
    int result=3;
    printf("[Process 3]: Dummy function in process 3 called.\n");
    printf("[Process 3]: Returning to process 2 with value:%d\n", result);
    syscall(SYS_pret, result);
    exit(0);
}

int main() {
    pid_t cpid = getpid();
    printf("[Process 3]: PID:%d\n",cpid);
    wait_for_call_via_loop();
    return 0;
}
