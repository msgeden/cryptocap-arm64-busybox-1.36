#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include <sys/syscall.h>
#include <sys/wait.h>

#include "cc_header.h"

int main(void)
{
    cc_dcl recv_call_grant;
    char char_shared='S';
    read(STDIN_FILENO, &recv_call_grant, sizeof(cc_dcl));
    printf("Caller:\t"); cc_print_dcl(recv_call_grant);
  
    cc_load_ver_dcl_to_DCLC(&recv_call_grant);
  
    cc_dcap sent_cap_char=cc_create_signed_cap_on_CR0(&char_shared, 0, sizeof(char), false);
    printf("Caller:\t"); cc_print_cap(sent_cap_char);
    
    char first_char='?';
    char second_char='?';
    char third_char='?';

    cc_dcall(); //CR0 will be foo argument  
    
    asm volatile (
        "mov %0, x0\t\n"   
        "mov %1, x1\t\n"   
        "mov %2, x2\t\n"   
        : "=r" (first_char), "=r" (second_char), "=r" (third_char)  // Output operand        
        :                   
        : "x0", "x1", "x2"
    );

    printf("Caller: foo returned with results:%c, %c, %c\n",first_char,second_char,third_char);
    printf("Caller: resuming parent (callee) process:\n\n\n");
    
    cc_resume_process(getppid());
    
    return 0;
}
