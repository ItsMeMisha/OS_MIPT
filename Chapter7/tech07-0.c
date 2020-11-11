/* Реализуйте программу, которая принимает два аргумента: CMD - команда для запуска, IN - имя файла, направляемого на ввод.

Программа должна запускать указанную команду, передавая ей на вход содежимое файла IN.

На стандартный поток вывода вывести количество байт, которое было записано запущенной командой в стандартный поток вывода. Вывод самой команды никуда выводить не нужно.
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

const size_t bufSize = 4 << 10;

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

size_t countOutput(int fd)
{
    char* buffer = (char*)calloc(sizeof(char), bufSize);

    size_t total = 0;
    for (ssize_t nextSize = read(fd, buffer, bufSize); nextSize != 0;
         nextSize = read(fd, buffer, bufSize))
        total += nextSize;

    free(buffer);
    return total;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
        return LACK_OF_ARGS;

    int pipeFds[2];
    if (pipe(pipeFds) != 0)
        return BAD_PIPE;

    int fileIn = open(argv[2], O_RDONLY);
    launch(argv[1], pipeFds, fileIn, pipeFds[1]);
    close(fileIn);
    close(pipeFds[1]);

    int status = 0;
    waitpid(-1, &status, 0);

    size_t numOfBytes = countOutput(pipeFds[0]);
    close(pipeFds[0]);

    printf("%zd\n", numOfBytes);
    return ALLRIGHT;
}
