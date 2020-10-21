#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>	// open
#include <sys/types.h>	// flags for open

enum {
    NotFound = 11,
    MapError = 22,
    AllocError = 33,
};

off_t fileSize(const char* filename);
void writeSpiral(char* buffer, int nLines, int width);
void printNum(char* buffer, int row, int column, int value, int width, int nLines);

int main(int argc,  char* argv[]) {
    if (argc != 4) {
        exit(NotFound);
    }

    int nLines = atoi(argv[2]);
    int width = atoi(argv[3]);

    off_t size = nLines * nLines * width + nLines;

    int fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0660);
    if (fd == -1) {
        perror("opening file");
        exit(NotFound);
    }

    /*  Does not compile :(
    if (fallocate(fd, 0, 0, size) != 0) {
        perror("allocating file");
        exit(AllocError);
    }
    */
    // make it more effective?

    size_t strLen = nLines * width + 1;
    char* bytes = (char*)calloc(nLines * width + 1, sizeof(char));
    for (int i = 0; i < nLines; ++i) {
        write(fd, bytes, strLen);
    }    
    free(bytes);

    char* buffer = mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        perror("mapping error");
        exit(MapError);
    }
    close(fd); 

    writeSpiral(buffer, nLines, width);

    munmap(buffer, size);
}

off_t fileSize(const char* filename) {
    struct stat fileStat;
    int res = lstat(filename, &fileStat);
    if (res != 0) {
        return -1;
    }
    return fileStat.st_size;
}

void printNum(char* buffer, int row, int column, int value, int width, int nLines) {
    //printf("%d moved by %d\n", value, width * (row * nLines + column) + row);
    char lastByte = *(buffer + width * (row * nLines + column) + row + width);
    //printf("%d at %d\n", lastByte, width * (row * nLines + column) + row + width);
    sprintf(buffer + width * (row * nLines + column) + row, "%*d", width, value);
    *(buffer + width * (row * nLines + column) + row + width) = lastByte;
    /*
    printf("wrote %d %d %d\n", *(buffer + width * (row * nLines + column) + row), 
                        *(buffer + width * (row * nLines + column) + row + 1),
                        *(buffer + width * (row * nLines + column) + row + 2));
    */
}

void writeSpiral(char* buffer, int nLines, int width) {
    int value = 1;
    for (int index = 0; index < nLines; index++) {
        for (int i = index; i < nLines - index; ++i) {
            printNum(buffer, index, i, value, width, nLines);
            //printf("a[%d, %d] = %d\n", index, i, value);
            ++value;
        }

        for (int i = index + 1; i < nLines - index; ++i) {
            printNum(buffer, i, nLines - index - 1, value, width, nLines);
            //printf("a[%d, %d] = %d\n", i, nLines - index - 1, value);
            ++value;
        }

        for (int i = nLines - index - 2; i >= index; --i) {
            printNum(buffer, nLines - index - 1, i, value, width, nLines);
            //printf("a[%d, %d] = %d\n", nLines - index - 1, i, value);
            ++value;
        }

        for (int i = nLines - index - 2; i > index; --i) {
            printNum(buffer, i, index, value, width, nLines);
            //printf("a[%d, %d] = %d\n", i, index, value);
            ++value;
        }
    }

    for (int i = 1; i <= nLines; ++i) {
        //printf("on %d symbol new line\n", i * nLines * width + (i - 1));
        *(buffer + i * nLines * width + (i - 1)) = '\n';
    }
}
