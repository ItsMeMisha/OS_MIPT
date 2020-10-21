#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

int main() {
    int error = 0;
    char* fileName = NULL;
    size_t nameLength = 0;

    off_t totalSize = 0;
    ssize_t read = getline(&fileName, &nameLength, stdin);

    while (read > 0) {
        if (fileName[read - 1] == '\n') {
            read--;
            fileName[read] = '\0';
        }
        
        struct stat fileInfo;
        error = lstat(fileName, &fileInfo);
        if (error == -1)
            return error;

        if (S_ISREG(fileInfo.st_mode))
            totalSize += fileInfo.st_size;

        read = getline(&fileName, &nameLength, stdin);
    }

    free (fileName);
    printf ("%ld", totalSize);
}
