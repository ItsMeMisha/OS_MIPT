#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>

enum Error {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    FILE_OPEN_ERROR = 2,
    MAP_ERROR = 3,
    UNMAP_ERROR = 4,
    BAD_FILE_SIZE = 5,
    ALLOC_ERROR = 6
};
const mode_t rightsForNewFiles = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

int setFileSize(int file, size_t Size);
void printNumInTable(char* buffer, int line, int coloumn, int width, int num, int size);
void setNewLines(char* buffer, int number, int width); 
void printSpiralInBuffer(char* buffer, const int number, const int width);

int main(int argc, char* argv[]) {
    if (argc < 4)
        return LACK_OF_ARGS;

    int output = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, rightsForNewFiles);
    if (output == -1)
        return FILE_OPEN_ERROR;

    int number = atoi(argv[2]);
    int width = atoi(argv[3]);
    const size_t Size = number * number * width + number;
    
    if (setFileSize(output, Size) == -1) {
        close(output);  
        return ALLOC_ERROR;
    } 

    char* buffer = mmap
        (NULL, Size, PROT_WRITE, MAP_SHARED, output, 0);
    if (buffer == MAP_FAILED) {
        close(output);
        return MAP_ERROR;
    }

    printSpiralInBuffer(buffer, number, width);

    int munmapResult = munmap(buffer, Size);
    close(output);

    if (munmapResult == -1)
        return UNMAP_ERROR;

    return ALLRIGHT;
}

int setFileSize(int file, const size_t Size) {
    size_t size = Size;
    char* buffer = (char*)calloc(size, sizeof(char));
    while (buffer == NULL && size > 0) {
        size /= 2;
        buffer = (char*)calloc(size, sizeof(char));
    }

    if (size <= 0)
        return -1;

    for (size_t i = 0; i < Size; i += size)
        write(file, buffer, size);

    write(file, buffer, Size % size);
    free(buffer);
    return 0;
}

void printNumInTable(char* buffer, int line, int coloumn, int width, int num, int size) {

    char* pos = buffer + line * size * width + width * coloumn + line;
    char prevVal = *(pos + width);
    sprintf(pos, "%*d", width, num);
    *(pos + width) = prevVal;
}

void setNewLines(char* buffer, int number, int width) {
    for (int i = 1; i <= number; ++i) {
        *(buffer + i * number * width + (i - 1)) = '\n';
    }        
}

void printSpiralInBuffer(char* buffer, const int number, const int width) {
    int curNum = 1;
    for (int layer = 0; layer <= number / 2; ++layer) {
        for (int i = layer; i < number - layer; ++i) {
            printNumInTable(buffer, layer, i, width, curNum, number);
            ++curNum;
        }

        for (int i = layer + 1; i < number - layer; ++i) {
            printNumInTable(buffer, i, number - layer - 1, width, curNum, number);
            ++curNum;
        }

        for (int i = number - layer - 2; i >= layer; --i) {
            printNumInTable(buffer, number - layer - 1, i, width, curNum, number);
            ++curNum;
        }

        for (int i = number - layer - 2; i > layer; --i) {
            printNumInTable(buffer, i, layer, width, curNum, number);
            ++curNum;
        }
    }
    setNewLines(buffer, number, width);
}

