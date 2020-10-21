#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
unsigned int processes = 1;
    do {
        pid = fork();
        if (pid == 0)
        processes ++;
    } while (-1 != pid);

    printf("Process %d reached out limit on processes %u\n", getpid(), processes);
    
    int status = 0;
    while (waitpid(-1, &status, 0) != -1);
    return 0;
}
