#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#define _FILE_OFFSET_BITS 64

enum Error {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    FILE_OPEN_ERROR = 2,
    BAD_FILE_SIZE = 3,
    CANNOT_GET_NEXT_ITEM = 4
};

struct Item {
    int value;
    uint32_t next_pointer;
};

int getNextItem(int fd, struct Item* item, uint32_t pointer) {
    int errcode = lseek(fd, pointer, SEEK_SET);
    if (errcode != -1)
        errcode = read(fd, item, sizeof(*item));
    return errcode;
}

off_t sizeOfFile(int fd) {
    off_t size = lseek(fd, 0, SEEK_END) - lseek(fd, 0, SEEK_SET);
    return size;
}

int main(int argc, char* argv[]) {
    if (argc < 2)
        return LACK_OF_ARGS;

    int input = open(argv[1], O_RDONLY);
    if (input == -1)
        return FILE_OPEN_ERROR;

    struct Item item = {0, 0};
    if (sizeOfFile(input) < sizeof(item))
        return BAD_FILE_SIZE;

    do {
        int errcode = getNextItem(input, &item, item.next_pointer);
        if (errcode == -1) {
            close(input);
            return CANNOT_GET_NEXT_ITEM;
        }

        printf ("%d ", item.value);
    } while (item.next_pointer != 0);

    close(input);
    return ALLRIGHT;
}
