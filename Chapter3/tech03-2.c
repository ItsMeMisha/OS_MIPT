#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

const mode_t Executable = S_IXUSR | S_IXGRP | S_IXOTH;

char checkELF(const char* fileName);
char checkIntrepretator(const char* fileName);

int main() {
    int error = 0;
    char* fileName = NULL;
    size_t nameLength = 0;

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

        if (fileInfo.st_mode & Executable)
            if (checkELF(fileName) == 0)
                if (checkIntrepretator(fileName) == 0)
                    printf("%s\n", fileName);
        
        read = getline(&fileName, &nameLength, stdin);
    }

    free (fileName);
}

char checkELF(const char* fileName) {
    FILE* file = fopen(fileName, "rb");
    const int bufferSize = 4;
    char buffer[bufferSize];
    
    int read = fread(buffer, sizeof(char), bufferSize, file);
    fclose(file);

    if (read != bufferSize) 
        return 0;
    
    if (buffer[0] == 0x7f && buffer[1] == 'E' 
        && buffer[2] == 'L' && buffer[3] == 'F')
            return 1;

    return 0;
}

char checkIntrepretator(const char* fileName) {
    FILE* file = fopen(fileName, "r");

    size_t pathLength = 0; 
    char* path = NULL;

    ssize_t read = getline(&path, &pathLength, file);
    fclose (file);

    if (read <= 0) {
        free(path);
        return 0;
    }

    if (path[read - 1] == '\n') {
        read--;
        path[read] = '\0';
    }

    if (pathLength < 3) {
        free(path);
        return 0;
    }

    if (path[0] != '#' && path[1] != '!') {
        free(path);
        return 0;
    }
    
    struct stat fileInfo;
    if (lstat(path + 2, &fileInfo) == -1) {
        free(path);
        return 0;
    }

    free(path);
    if (fileInfo.st_mode & Executable)
        return 1;

    return 0;
}

