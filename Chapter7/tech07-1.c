/* Программе передаётся два аргумента: CMD1 и CMD2. Необходимо запустить два процесса, выполняющих эти команды, и перенаправить стандартный поток вывода CMD1 на стандартный поток ввода CMD2.

В командной строке это эквивалентно CMD1 | CMD2.

Родительский процесс должен завершаться самым последним!
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef enum {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    BAD_PIPE = 2,
    BAD_EXEC = 3,
    BAD_FORK = 4
} Error;

void launch(const char* cmd, int pipeFds[2], int inFd, int outFd)
{
    pid_t pid = fork();
    if (pid == -1)
        exit(BAD_FORK);

    if (pid > 0)
        return;

    dup2(inFd, 0);
    dup2(outFd, 1);

    if (inFd != 0)
        close(inFd);
    if (outFd != 1)
        close(outFd);

    if (pipeFds[0] != inFd)
        close(pipeFds[0]);
    if (pipeFds[1] != outFd)
        close(pipeFds[1]);

    execlp(cmd, cmd, NULL);
    exit(BAD_EXEC);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
        return LACK_OF_ARGS;

    int pipeFds[2];
    if (pipe(pipeFds) != 0)
        return BAD_PIPE;

    launch(argv[1], pipeFds, 0, pipeFds[1]);
    close(pipeFds[1]);

    launch(argv[2], pipeFds, pipeFds[0], 1);
    close(pipeFds[0]);

    int status = 0;
    waitpid(-1, &status, 0);
    waitpid(-1, &status, 0);
    return ALLRIGHT;
}
