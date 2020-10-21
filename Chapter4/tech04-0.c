#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

enum Error {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    FILE_OPEN_ERROR = 2,
    MAP_ERROR = 3,
    UNMAP_ERROR = 4
};

int main(int argc, char* argv[]) {
    if (argc < 3)
        return LACK_OF_ARGS;

    int input = open(argv[1], O_RDONLY);
    if (input == -1)
        return FILE_OPEN_ERROR;

    struct stat fileInfo;
    fstat(input, &fileInfo);

    char* buffer = mmap(NULL, fileInfo.st_size, PROT_READ, MAP_SHARED, input, 0);
    if (buffer == MAP_FAILED) {
        close(input);
        return MAP_ERROR;
    }

    char* pos = buffer;
    while (pos != NULL) {
        pos = strstr(pos, argv[2]);

        if (pos != NULL) {
            long shift = pos - buffer;
            printf ("%ld ", shift);
            pos++;
        }
    }

    int munmapResult = munmap(buffer, fileInfo.st_size);
    close(input);

    if (munmapResult == -1)
        return UNMAP_ERROR;

    return ALLRIGHT;
}
