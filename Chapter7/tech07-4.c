/* Программе передаётся произвольное количество аргументов: CMD1, CMD2, ..., CMDN.

Необходимо реализовать эквивалент запуска их командной строки: CMD1 | CMD2 | ... | CMDN.

Родительский процесс должен завершаться самым последним!

В отличие от предыдущей задачи, разрешается использовать только два канала. Допускается использовать не более 8 открытых файловых дескрипторов.

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

void launch(const char* cmd, int twoFdToClose[2], int inFd, int outFd)
{
    pid_t pid = fork();
    if (pid == -1)
        exit(BAD_FORK);

    if (pid > 0) {
        int status = 0;
        waitpid(pid, &status, 0);
        return;
    }

    dup2(inFd, 0);
    dup2(outFd, 1);

    if (inFd != 0 && inFd != 1)
        close(inFd);
    if (outFd != 1 && inFd != 0)
        close(outFd);

    if (twoFdToClose[0] != inFd)
        close(twoFdToClose[0]);
    if (twoFdToClose[1] != outFd)
        close(twoFdToClose[1]);

    execlp(cmd, cmd, NULL);
    exit(BAD_EXEC);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return ALLRIGHT;

    int firstPipeFds[2];
    if (pipe(firstPipeFds) != 0)
        return BAD_PIPE;

    if (argc == 2) {
        launch(argv[1], firstPipeFds, 0, 1);
        close(firstPipeFds[0]);
        close(firstPipeFds[1]);
        return ALLRIGHT;
    }

    launch(argv[1], firstPipeFds, 0, firstPipeFds[1]);
    close(firstPipeFds[1]);

    int secondPipeFds[2];
    int twoFdToClose[2] = {0, 0};
    for (int i = 2; i < argc - 1; ++i) {
        if (pipe(secondPipeFds) != 0)
            return BAD_PIPE;

        twoFdToClose[0] = secondPipeFds[0];
        twoFdToClose[1] = secondPipeFds[1];

        launch(argv[i], twoFdToClose, firstPipeFds[0], secondPipeFds[1]);
        close(firstPipeFds[0]);
        close(secondPipeFds[1]);
        firstPipeFds[0] = secondPipeFds[0];
    }

    twoFdToClose[0] = firstPipeFds[0];
    twoFdToClose[1] = 1;

    launch(argv[argc - 1], twoFdToClose, firstPipeFds[0], 1);
    close(firstPipeFds[0]);

    return ALLRIGHT;
}
