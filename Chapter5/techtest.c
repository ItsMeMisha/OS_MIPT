#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{

    unsigned int numOfProcesses = 1;
    pid_t pid = 0;
    int status = 0;

    while ((pid = fork()) != -1) {
        if (pid == 0)
            exit(0);
        numOfProcesses++;
    }

    for (unsigned int i = 0; i < numOfProcesses; ++i)
        waitpid(-1, &status, 0);

    printf("%u", numOfProcesses);
    return 0;
}
