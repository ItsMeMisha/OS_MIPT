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

    size_t numOfBytes = countOutput(0);

    printf("%zd\n", numOfBytes);
    return ALLRIGHT;
}
