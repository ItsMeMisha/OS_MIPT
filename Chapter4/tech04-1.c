#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>

enum Error {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    FILE_OPEN_ERROR = 2,
    MAP_ERROR = 3,
    UNMAP_ERROR = 4,
    BAD_FILE_SIZE = 5,
    CANNOT_GET_NEXT_ITEM = 6
};

struct Item {
    int value;
    uint32_t next_pointer;
};

void printList(struct Item* list);

int main(int argc, char* argv[]) {
    if (argc < 2)
        return LACK_OF_ARGS;

    int input = open(argv[1], O_RDONLY);
    if (input == -1)
        return FILE_OPEN_ERROR;

    struct stat fileInfo;
    fstat(input, &fileInfo);

    if (fileInfo.st_size < sizeof(struct Item)) {
        close(input);
        return BAD_FILE_SIZE;
    }

    struct Item* buffer = mmap(NULL, fileInfo.st_size, PROT_READ, MAP_SHARED, input, 0);
    if (buffer == MAP_FAILED) {
        close(input);
        return MAP_ERROR;
    }

    printList(buffer);

    int munmapResult = munmap(buffer, fileInfo.st_size);
    close(input);

    if (munmapResult == -1)
        return UNMAP_ERROR;

    return ALLRIGHT;
}

void printList(struct Item* list) {
    uint32_t curPos = 0;
    do {
        printf("%d ", list[curPos].value);
        curPos = list[curPos].next_pointer / sizeof(*list);
    } while (curPos != 0);
}

