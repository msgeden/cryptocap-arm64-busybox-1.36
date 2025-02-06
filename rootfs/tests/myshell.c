#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAXLINE 1024
#define MAXARGS 64

// Trim leading and trailing whitespace
char* trim_whitespace(char* str) {
    char *end;
    // Trim leading space
    while (*str && (*str == ' ' || *str == '\t' || *str == '\n'))
        str++;
    if (*str == 0)  // All spaces?
        return str;
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n'))
        *end-- = '\0';
    return str;
}

// Parse a command string into arguments for execvp
int parse_command(char* cmd, char* argv[]) {
    int argc = 0;
    cmd = trim_whitespace(cmd);
    char *token = strtok(cmd, " ");
    while (token != NULL && argc < MAXARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    return argc;
}

int main(void) {
    char line[MAXLINE];

    while (1) {
        // Print prompt and read input
        printf("myshell> ");
        if (!fgets(line, sizeof(line), stdin))
            break;
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;
        // Exit condition
        if (strcmp(line, "exit") == 0)
            break;

        // Check if there is a pipe symbol
        char *pipe_pos = strchr(line, '|');
        if (pipe_pos != NULL) {
            // Split into two commands by replacing '|' with a null terminator.
            *pipe_pos = '\0';
            char *cmd1 = trim_whitespace(line);
            char *cmd2 = trim_whitespace(pipe_pos + 1);

            char *argv1[MAXARGS], *argv2[MAXARGS];
            parse_command(cmd1, argv1);
            parse_command(cmd2, argv2);

            int fd[2];
            if (pipe(fd) == -1) {
                perror("pipe");
                continue;
            }

            // Fork first child to execute cmd1
            pid_t pid1 = fork();
            if (pid1 < 0) {
                perror("fork");
                continue;
            }
            if (pid1 == 0) {
                // Redirect stdout to the pipe’s write end.
                if (dup2(fd[1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(fd[0]);
                close(fd[1]);
                execvp(argv1[0], argv1);
                perror("execvp");
                exit(EXIT_FAILURE);
            }

            // Fork second child to execute cmd2
            pid_t pid2 = fork();
            if (pid2 < 0) {
                perror("fork");
                continue;
            }
            if (pid2 == 0) {
                // Redirect stdin to the pipe’s read end.
                if (dup2(fd[0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(fd[1]);
                close(fd[0]);
                execvp(argv2[0], argv2);
                perror("execvp");
                exit(EXIT_FAILURE);
            }

            // Parent closes both ends of the pipe and waits for children.
            close(fd[0]);
            close(fd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        } else {
            // No pipe: execute the command normally.
            char *argv[MAXARGS];
            parse_command(line, argv);
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                continue;
            }
            if (pid == 0) {
                execvp(argv[0], argv);
                perror("execvp");
                exit(EXIT_FAILURE);
            }
            waitpid(pid, NULL, 0);
        }
    }

    return 0;
}