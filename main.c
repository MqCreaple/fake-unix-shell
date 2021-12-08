#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

int main() {
    size_t MAX_SHELL_LENGTH = 100;
    char *toks[MAX_SHELL_LENGTH >> 1 + 1]; unsigned top = 0;
    char stats[MAX_SHELL_LENGTH >> 1 + 1]; // 1: foreground; 0: background
    char **commands[MAX_SHELL_LENGTH >> 1]; unsigned commandNum = 0; // the number of commands
    char shouldContinue = 1; // when running 'exit' command, set 'shouldContinue' to zero
    char error = 0; // 1 when there is an error when executing
    while(shouldContinue) {
        /** initialize parameters */
        memset(toks, 0, (MAX_SHELL_LENGTH >> 1 + 1) * sizeof(char*));
        memset(commands, 0, (MAX_SHELL_LENGTH >> 1) * sizeof(char**));
        top = 0; commandNum = 0;
        printf("%s", SHELL_PROMPT);
        /** get all tokens */
        char* line = malloc(MAX_SHELL_LENGTH);
        getline(&line, &MAX_SHELL_LENGTH, stdin);
        toks[top++] = strtok(line, " \n");
        while(toks[top - 1] != NULL) {
            toks[top++] = strtok(NULL, " \n");
        }
        top--;
        /** extract commands */
        for(unsigned i = 0; i < top; i++) {
            if(strcmp(toks[i], "&") == 0) {
                if(commands[commandNum] == NULL) {
                    printf("Syntax error: invalid \'&\'\n");
                    error = 1;
                    break;
                }
                toks[i] = NULL;
                stats[commandNum] = 0;
                commandNum++;
            } else if(strcmp(toks[i], ";") == 0) {
                if(commands[commandNum] == NULL) {
                    printf("Syntax error: invalid \';\'\n");
                    error = 1;
                    break;
                }
                toks[i] = NULL;
                stats[commandNum] = 1;
                commandNum++;
            } else if(commands[commandNum] == NULL) {
                commands[commandNum] = toks + i;
            }
        }
        // When there is an error, start a new loop.
        if(error) {
            error = 0;
            free(line);
            continue;
        }
        // If the command has no special characters in the end, treat it as a foreground process.
        if(commands[commandNum] != NULL) {
            stats[commandNum] = 1;
            commandNum++;
        }
        /** execute */
        for(unsigned i = 0; i < commandNum; i++) {
            char** str = commands[i];
            // If the command is 'exit', break from the loop and return.
            if(strcmp(*str, "exit") == 0) {
                shouldContinue = 0;
                break;
            }
            // create a child process to exit the command
            int pid = fork();
            if(pid < 0) {
                printf("Process Creation failed!\n");
                break;
            } else if(pid > 0) {
                if(stats[i]) {
                    int stat;
                    wait(&stat);
                } else {
                    printf("Background process assigned. PID: %d\n", pid);
                }
            } else {
                execvp(str[0], str);
                exit(0);
            }
            puts("");
        }
        /** destruct */
        free(line);
    }
}