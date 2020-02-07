#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

void MemoryCheck(void *pointer) {
    if (!pointer) {
        perror("Memory error");
        exit(1);
    }
}

void PrintError(char *string) {
    perror(string);
    exit(1);
}

void Conveyor(int argc, char **argv) {
    int programmes_number = argc - 1;
    int pipes_number = argc - 2;
    int **pipes = (int **) malloc(sizeof(int *) * pipes_number);
    MemoryCheck(pipes);

    for (int i = 0; i < pipes_number; ++i) {
        pipes[i] = (int *) malloc(sizeof(int) * 2);
        MemoryCheck(pipes[i]);

        if (pipe(pipes[i]) == -1) {
            PrintError("Pipe error");
        }
    }

    for (int i = 0; i < programmes_number; ++i) {
        pid_t p_id = fork();

        if (p_id == -1) {
            PrintError("Fork error");
        }

        if (!p_id) {

            if (i != 0) {
                if (dup2(pipes[i - 1][0], 0) == -1) {
                    PrintError("dup error");
                }
            }

            if (i != programmes_number - 1) {
                if (dup2(pipes[i][1], 1) == -1) {
                    PrintError("dup error");
                }
            }

            for (int j = 0; j < pipes_number; ++j) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            if (execlp(argv[i + 1], argv[i + 1], NULL) == -1) {
                PrintError(argv[i + 1]);
            }
        }
    }

    for (int j = 0; j < pipes_number; ++j) {
        close(pipes[j][0]);
        close(pipes[j][1]);
    }

    for (int j = 0; j < programmes_number; ++j) {
        if (wait(0) == -1) {
            PrintError("Wait error");
        }
    }
}

int main(int argc, char **argv) {
    Conveyor(argc, argv);
    return 0;
}