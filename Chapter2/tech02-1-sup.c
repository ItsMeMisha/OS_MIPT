#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

enum Error {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    FILE_OPEN_ERROR = 2,
    BAD_FILE_SIZE = 3,
    CANNOT_GET_NEXT_ITEM = 4
};

typedef int Item;

int getNextItem(int fd, Item* item) {
        int errcode = read(fd, item, sizeof(*item));
    return errcode;
}

int sizeOfFile(int fd) {
    int size = lseek(fd, 0, SEEK_END) - lseek(fd, 0, SEEK_SET);
    return size;
}

int main(int argc, char* argv[]) {
    if (argc < 2)
        return LACK_OF_ARGS;

    int input = open(argv[1], O_RDONLY);
    if (input == -1)
        return FILE_OPEN_ERROR;

    Item item = {0};
    if (sizeOfFile(input) < sizeof(item))
        return BAD_FILE_SIZE;

    int errcode = 0;

    do {
        errcode = getNextItem(input, &item);
        if (errcode == -1) {
            close(input);
            return CANNOT_GET_NEXT_ITEM;
        }

        printf ("%d ", item);
    } while (errcode > 0);

    return ALLRIGHT;
}
