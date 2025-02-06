#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <ctype.h>

int main(void) {
    int size;
    read(STDIN_FILENO, &size, sizeof(int));
    //scanf("%d", &size); 
    char* buffer=(char*)malloc(size*sizeof(char));
    //scanf("%s", buffer); 
    read(STDIN_FILENO, buffer, size);
    printf("Hello lowercase: %s\n", buffer);
    for (int i = 0; i<size-1; i++) {
        buffer[i] = toupper(buffer[i]); // Convert to uppercase
    }
    printf("Hello uppercase: %s\n", buffer);
    printf("\n");
    free(buffer);
    return 0;
}

