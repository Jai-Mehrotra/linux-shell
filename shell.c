#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
 *
 */


char **tokenize(char *line)
{
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for (i = 0; i < strlen(line); i++)
    {

        char readChar = line[i];

        if (readChar == ' ' || readChar == '\n' || readChar == '\t')
        {
            token[tokenIndex] = '\0';
            if (tokenIndex != 0)
            {
                tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
                strcpy(tokens[tokenNo++], token);
                tokenIndex = 0;
            }
        }
        else
        {
            token[tokenIndex++] = readChar;
        }
    }

    free(token);
    tokens[tokenNo] = NULL;
    return tokens;
}
void sigint_handler(int sig)
{
    pid_t pid = waitpid(-1, NULL, WNOHANG);
    if (pid > 0)
    {
        printf("\nChild process %d was terminated by signal %d\n", pid, sig);
    }
}

int main(int argc, char *argv[])
{
    struct sigaction act;
    act.sa_handler = &sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    char line[MAX_INPUT_SIZE];
    char **tokens;
    int i;

    while (1)
    {
        /* BEGIN: TAKING INPUT */
        bzero(line, sizeof(line));
        printf("$ ");
        scanf("%[^\n]", line);
        getchar();

        // printf("Command entered: %s (remove this debug output later)\n", line);
        /* END: TAKING INPUT */

        line[strlen(line)] = '\n'; // terminate with new line
        tokens = tokenize(line);

        int background = 0;
        int last_token = 0;
        while (tokens[last_token] != NULL)
        {
            last_token++;
        }
        last_token--;
        if (strcmp(tokens[last_token], "&") == 0)
        {
            background = 1;
            tokens[last_token] = NULL;
        }
        if (strcmp(tokens[0], "cd") == 0)
        {
            if (tokens[1] == NULL)
            {
                fprintf(stderr, "Error: expected argument to \"cd\"\n");
            }
            else
            {
                if (chdir(tokens[1]) != 0)
                {
                    perror("Error");
                }
            }
        }
        else if (strcmp(tokens[0], "exit") == 0)
        {
            exit(0);
        }

        // fork a new process to run the command
        pid_t pid = fork();

        if (pid == 0)
        {
            // Call the command using the execvp function
            execvp(tokens[0], tokens);
            perror("Error");
            exit(1);
        }
        else if (pid > 0)
        {
            // parent process
            if (!background)
            {
                waitpid(pid, NULL, 0);
            }
        }
        else
        {
            perror("Error");
        }
        
        for (i = 0; tokens[i] != NULL; i++)
        {
            free(tokens[i]);
        }
        free(tokens);
    }
    return 0;
}

