#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _FILE_OFFSET_BITS 64

enum Error {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    FILE_OPEN_ERROR = 2,
    BAD_ALLOC = 3,
    BAD_READ = 4,
    BAD_WRITE = 5,
    UNABLE_CREATE_FILE = 6
};

enum Error error = ALLRIGHT;

const unsigned int bufferSize = 1 << 10 << 10; //4 Mb of int32
const mode_t rightsForNewFiles = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

off_t sizeOfFile(int fd); 
void sortFile(int input);
void splitFileAndSort(int input, int* outputFiles, int numOfFiles);
int createFile(int fileNumber, const char* baseName);
void mergeAllFiles(int* files, int output, int numOfFiles);
void mergeFiles(int fileSource1, int fileSource2, int fileResult);
void closeAllFiles(int* files, int numOfFiles);
char* newFileName(int fileNumber, const char* baseName);
int comparator(const void* elem1, const void* elem2);

int main(int argc, char* argv[]) {
    if (argc < 2)
        return LACK_OF_ARGS;

    int input = open(argv[1], O_RDWR);
    if (input == -1)
        return FILE_OPEN_ERROR;

    sortFile(input);
    
    close (input);
    return error;
}

off_t sizeOfFile(int fd) {
    off_t size = lseek(fd, 0, SEEK_END) - lseek(fd, 0, SEEK_SET);
    return size;
}

void sortFile(int input) {
    off_t inputSize = sizeOfFile(input);
    int numOfSupportFiles = inputSize / (bufferSize * sizeof(int));
    if (inputSize % (bufferSize * sizeof(int)) != 0)
        ++numOfSupportFiles;

    int* outputFiles = (int*)calloc(numOfSupportFiles, sizeof(int));
    if (outputFiles == NULL) {
        error = BAD_ALLOC;
        return;
    }

    splitFileAndSort(input, outputFiles, numOfSupportFiles);
    if (error != ALLRIGHT)
        return;

    if (numOfSupportFiles > 1) {
        mergeAllFiles(outputFiles, input, numOfSupportFiles);
        if (error != ALLRIGHT)
            return;
    }
    free(outputFiles);
}

void splitFileAndSort(int input, int* outputFiles, int numOfFiles) {
    int* buffer = (int*)calloc(sizeof(int), bufferSize);

    for (int i = 0; i < numOfFiles; ++i) {
        int readNum = read(input, buffer, bufferSize * sizeof(int));

        if (readNum == -1) {
            error = BAD_READ;
            closeAllFiles(outputFiles, i - 1);
            return;
        }

        qsort(buffer, readNum / sizeof(int), sizeof(int), comparator);

        if (numOfFiles == 1) {
            lseek(input, 0, SEEK_SET);
            int written = write(input, buffer, readNum);
            if (written == -1) {
                error = BAD_WRITE;
                closeAllFiles(outputFiles, i - 1);
                return;    
            }
        free(buffer);
        return;
        }
                  
        outputFiles[i] = createFile(i, "SUPPORT_FILE-");
        if (outputFiles[i] == -1) {
            error = UNABLE_CREATE_FILE;
            closeAllFiles(outputFiles, i - 1);
            return;
        }

        int written = write(outputFiles[i], buffer, readNum);
        if (written == -1) {
            error = BAD_WRITE;
            closeAllFiles(outputFiles, i - 1);
            return;    
        }
    }
    free(buffer);
}

int createFile(int fileNumber, const char* baseName) {
    char* name = newFileName(fileNumber, baseName);
    int fd = open(name, O_RDWR | O_CREAT, rightsForNewFiles);
    free(name);
    return fd;
}

void mergeAllFiles(int* files, int output, int numOfFiles) {
    int bufferFile = createFile(numOfFiles + 1, "BUFFER_FILE-");
    int filesRemain = numOfFiles;

    while (filesRemain > 2) {
        int currentFreeFileIndx = 0;
        for (int i = 0; i < filesRemain; i += 2) {
            if (currentFreeFileIndx == 0) {
                mergeFiles(files[i], files[i + 1], bufferFile);
                int bufFD = files[i];
                files[i] = bufferFile;
                bufferFile = bufFD;
                currentFreeFileIndx = i + 1;
            } else {
                mergeFiles(files[i], files[i + 1], files[currentFreeFileIndx]);
                ++currentFreeFileIndx;
            }
            filesRemain = filesRemain / 2 + filesRemain % 2;
        }
    }
    close(bufferFile);
    if (numOfFiles > 1)
        mergeFiles(files[0], files[1], output);
}

void mergeFiles(int fileSource1, int fileSource2, int fileResult) {
    int number1 = 0;
    int number2 = 0;
    int* remainNumbers = (int*)calloc(sizeof(int), bufferSize);
    lseek(fileResult, 0, SEEK_SET);
    lseek(fileSource1, 0, SEEK_SET);
    lseek(fileSource2, 0, SEEK_SET);
    int written = 0;

    int read1 = read(fileSource1, &number1, sizeof(number1));
    int read2 = read(fileSource2, &number2, sizeof(number2));
    if (read1 == -1 || read2 == -1) {
        error = BAD_READ;
        return;
    }

    while (read1 == sizeof(number1) && read2 == sizeof(number2)) {
        if (number1 < number2) {
            written = write(fileResult, &number1, sizeof(number1));
            read1 = read(fileSource1, &number1, sizeof(number1));
        } else {
            written = write(fileResult, &number2, sizeof(number2));
            read2 = read(fileSource2, &number2, sizeof(number2));
        }

        if (written == -1) {
            error = BAD_WRITE;
            return;
        }
    }

    int remainFile = 0;
    if (read1 != sizeof(number1))
        remainFile = fileSource2;
    else if (read2 != sizeof(number2))
        remainFile = fileSource1;

    if (remainFile != 0) {
        do {
            read1 = read(remainFile, remainNumbers, bufferSize * sizeof(remainNumbers[0]));
            written = write(fileResult, remainNumbers, read1);

            if (written == -1) {
                error = BAD_WRITE;
                return;
            }
        } while (read1 == bufferSize * sizeof(remainNumbers[0]));
    }
    free (remainNumbers);
}

void closeAllFiles(int* files, int numOfFiles) {
    for (int i = 0; i < numOfFiles; ++i)
        close(files[i]);
}

char* newFileName(int fileNumber, const char* baseName) {
    if (baseName == NULL)
        baseName = "STD_NAME";

    const int maxIntLength = 10;
    char* newName = (char*)calloc(sizeof(char), strlen(baseName) + maxIntLength);
    snprintf (newName, strlen(baseName) + maxIntLength, "%s%d", baseName, fileNumber);
    return newName;
}

int comparator(const void* elem1, const void* elem2) {
    return ( *(int*) elem1 - *(int*) elem2);
}

