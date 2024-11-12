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
int main(int argc, char *argv[])
{
    char line[MAX_INPUT_SIZE];
    char *tokens[MAX_TOKEN_SIZE];
    char *command;
    int i, j, k, last_token, background;
    int success;
    pid_t pid;
    while (1)
    {
        // Take input
        bzero(line, sizeof(line));
        printf("$ ");
        scanf("%[^\n]", line);
        getchar();

        // Tokenize the input
        k = 0;
        command = strtok(line, "&&");
        while (command != NULL)
        {
            // Add command to tokens array
            tokens[k++] = strdup(command);
            command = strtok(NULL, "&&");
        }
        tokens[k] = NULL;

        // Loop through each command separated by "&&" or "&&&"
        for (i = 0; tokens[i] != NULL; i++)
        {
            char *newtokens[MAX_TOKEN_SIZE];
            success = 0;

            // Check if the command should be executed in parallel
            background = 0;
            if (strstr(tokens[i], "&&&") != NULL)
            {
                background = 1;
                tokens[i] = strtok(tokens[i], "&&&");
            }

            // Tokenize the current command
            last_token = 0;
            command = strtok(tokens[i], " ");
            while (command != NULL)
            {
                newtokens[last_token++] = strdup(command);
                command = strtok(NULL, " ");
            }
            newtokens[last_token] = NULL;
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

            // Check if the command is "exit"
            if (strcmp(newtokens[0], "exit") == 0)
            {
                exit(0);
            }

            // Fork a new process
            pid = fork();

            if (pid == 0)
            {
                // Child process
                execvp(newtokens[0], newtokens);
                perror("Error");
                exit(1);
            }
            else if (pid > 0)
            {
                // Parent process
                if (!background)
                {
                    waitpid(pid, &success, 0);
                }
            }
            else
            {
                perror("Error");
            }

            // Check if the previous command was successful
            if (success != 0)
            {
                break;
            }

            // Free memory used by newtokens array
            for (j = 0; newtokens[j] != NULL; j++)
            {
                free(newtokens[j]);
            }
        }

        // Free memory used by tokens array
        for (j = 0; tokens[j] != NULL; j++)
        {
            free(tokens[j]);
        }
    }
    return 0;
}
