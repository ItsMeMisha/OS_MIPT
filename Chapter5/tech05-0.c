#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    size_t numOfProcesses = 1;
    int status = 0;
    pid_t pid = 0;

    while ((pid = fork()) != -1) {
        if (pid == 0)
            ++numOfProcesses;
        else if (pid > 0) {
            waitpid(-1, &status, 0);
            exit(0);
        }
    }

    printf("%lu\n", numOfProcesses);
    exit(0);
}
