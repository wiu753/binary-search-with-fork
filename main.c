#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define READ 0
#define WRITE 1

// Change this to input from terminal, should also be capital if not coming from termial

int debugger = 0;
int generateRandomNumber();
int pipeIsOk(int pipefd[]);
int createAndCheckFork(int *pid);
void parentAction(int pipefd[], int pipefd2[]);
void childAction(int pipefd[], int pipefd2[]);

// Time complexity: O(log(n))
// Space complecity: O(1)
int main(int argc, char **argv)
{
    if(argc > 1)
        debugger = atoi(argv[1]); // if 1 is specified in cmd then debugger runs

    int pipefd[2], pipefd2[2];
    pid_t pid;

    if (!pipeIsOk(pipefd) || !pipeIsOk(pipefd2))
        return 1;

    if (!createAndCheckFork(&pid))
        return 1;

    if (pid > 0)
        parentAction(pipefd, pipefd2);
    else
        childAction(pipefd, pipefd2);
}

// Helper functions

int generateRandomNumber()
{
    srand(time(NULL));
    return rand() % 1001;
}

int pipeIsOk(int pipefd[])
{
    if (pipe(pipefd) == -1)
    {
        fprintf(stderr, "Pipe failed");
        return 0;
    }
    else
        return 1;
}

int createAndCheckFork(int *pid)
{
    *pid = fork();

    if (*pid < 0)
    {
        fprintf(stderr, "Fork failed");
        return 0;
    }
    else
        return 1;
}

// Parent and child actions

void parentAction(int pipefd[], int pipefd2[])
{
    int initialGuess = 500, responseFromChild, min = 0, max = 999, guess, numberOfGuesses = 0;

    close(pipefd[READ]);   // The parent is not going to read from the first pipe, close the read end of the pipe
    close(pipefd2[WRITE]); // The parent is not going to write to the second pipe, close the write end of the pipe

    while (1337)
    {
        guess = (int)floor(((min + max) / 2));

        write(pipefd[WRITE], &guess, sizeof(int));

        if (debugger == 1)
            printf("Parent: guesses value : %d\n", ((min + max) / 2));

        // If the child closes the write end of the second pipe, break out of the loop
        if (read(pipefd2[READ], &responseFromChild, sizeof(responseFromChild)) > 0)
        {
            numberOfGuesses++;
            if (debugger == 1)
                printf("Parent: reads value : %d\n", responseFromChild);
            if (responseFromChild == 0)
                break;
            else if (responseFromChild == 1)
                min = ((min + max) / 2) + 1;
            else if (responseFromChild == -1)
                max = ((min + max) / 2) - 1;
        }
        else
            break;
    }

    printf("The algorithm guessed %d in %d tries", guess, numberOfGuesses);
    close(pipefd[WRITE]); // Close the write end of the first pipe
    close(pipefd2[READ]); // Close the read end of the second pipe
}

void childAction(int pipefd[], int pipefd2[])
{
    int correctNumber = generateRandomNumber(), parentsGuess, responseToParent;
    printf("The child chose number %d \n", correctNumber);
    close(pipefd[WRITE]); // The child is not going to write to the first pipe, close the write end of the pipe.
    close(pipefd2[READ]); // The child is not going to read from the second pipe, close the read end of the pipe.

    while (1337)
    {
        // If the parent closes the write end of the first pipe, break out of the loop
        if (read(pipefd[READ], &parentsGuess, sizeof(parentsGuess)) > 0)
        {
            if (debugger == 1)
                printf("Child: read value : %d\n", parentsGuess);
        }
        else
            break;

        if (parentsGuess == correctNumber)
            responseToParent = 0; // Correct
        else if (parentsGuess < correctNumber)
            responseToParent = 1; // Too low, go up
        else if (parentsGuess > correctNumber)
            responseToParent = -1; // Too high, go down

        write(pipefd2[WRITE], &responseToParent, sizeof(responseToParent));

        if (debugger == 1)
            printf("Child: responds with : %d\n", responseToParent);
    }

    close(pipefd[READ]);   // Close the read end of the first pipe
    close(pipefd2[WRITE]); // Close the write end of the second pipe
}