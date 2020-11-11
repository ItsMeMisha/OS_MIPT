/* Программе на стандартном потоке ввода передаётся список имён файлов.

Если файл является символической ссылкой, то нужно вывести абсолютное имя того файла, на который эта ссылка указывает.

Если файл является регулярным, то необходимо создать символическую ссылку в текущем каталоге, приписав в названии файла префикс link_to_.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

int createLink(const char* fileName, size_t length);
void printAbsoluteName(const char* linkName);
const char* getNameFromPath(const char* path);
char* getNameFromNonConstPath(char* path); 
void normalize_path(char* path);
 
int main() {
    int error = 0;
    char* fileName = NULL;
    size_t nameLength = 0;

    ssize_t read = getline(&fileName, &nameLength, stdin);

    while (read > 0) {
        if (fileName[read - 1] == '\n') {
            fileName[read - 1] = '\0';
        }
        
        struct stat fileInfo;
        error = lstat(fileName, &fileInfo);
        if (error == -1)
            return error;

        if (S_ISLNK(fileInfo.st_mode))
            printAbsoluteName(fileName);

        if (S_ISREG(fileInfo.st_mode)) {
            const char* name = getNameFromPath(fileName);
            if (error = createLink(name, strnlen(name, NAME_MAX)) == -1) {
                free(fileName);
                return error;
            }
        }

        read = getline(&fileName, &nameLength, stdin);
    }

    free (fileName);
}

int createLink(const char* fileName, size_t length) {
    const char linkPrefix[] = "link_to_";
    char* linkName = (char*) calloc(length + sizeof(linkPrefix), sizeof(char));

    strncpy(linkName, linkPrefix, sizeof(linkPrefix));
    strncpy(linkName + sizeof(linkPrefix) - 1, fileName, length);

    int error = symlink (fileName, linkName);
    free(linkName);
    return error;
}

void printAbsoluteName(const char* linkName) {
    size_t bufferSize = 2 * PATH_MAX;
    char* buffer = (char*) calloc (bufferSize, sizeof(char));
    char* linkPart = buffer;
    
    if (*linkName == '/') {
        strncpy(buffer, linkName, PATH_MAX);
        linkPart = getNameFromNonConstPath(buffer);
   
    } else {
        buffer = getcwd(buffer, bufferSize);
        while (*linkPart != '\0') {
            linkPart++;
        }
        *linkPart = '/';
        strncpy(linkPart + 1, linkName, PATH_MAX);
        linkPart = getNameFromNonConstPath(buffer);
    }

    ssize_t linkPartLength = readlink(linkName, linkPart, bufferSize / 2);
    if (linkPartLength == -1) {
        free(buffer);
        return;
    }
    *(linkPart + linkPartLength) = '\0';
    
    if (*linkPart == '/')
        printf ("%s\n", linkPart);
    else {
        normalize_path(buffer);
        printf ("%s\n", buffer);
    }

    free(buffer);
}

const char* getNameFromPath(const char* path) {
    const char* nextSlash = path - 1;

    while (nextSlash != NULL) {
        path = nextSlash + 1;
        nextSlash = strchr(path, '/');
    }
    return path;
}

char* getNameFromNonConstPath(char* path) {
    char* nextSlash = path - 1;

    while (nextSlash != NULL) {
        path = nextSlash + 1;
        nextSlash = strchr(path, '/');
    }
    return path;
}

void normalize_path(char* path) {
    char* currentPtr = path;
    char* normalPtr = path;
    char* beginning = path;
 
    if (*currentPtr == '/') {
        currentPtr++;
        normalPtr++;
    }

    while (*currentPtr != '\0') {
        while (*currentPtr == '/' && *currentPtr != '\0')
            currentPtr++;

        while (*currentPtr == '.') {
            if (*(currentPtr + 1) == '/')
                currentPtr += 2;

            if (*(currentPtr + 1) == '.')
                if (*(currentPtr + 2) == '/' || *(currentPtr + 2) == '\0') {
                    if (*(normalPtr - 1) == '/' && (normalPtr - 1) != beginning) {
                        normalPtr -= 2;

                        while (normalPtr != beginning && *(normalPtr - 1) != '/')
                            normalPtr--;
                    }

                    if (*(currentPtr + 2) != '\0')
                        currentPtr += 3;
                    else currentPtr += 2;
                }
        }

        while (*currentPtr != '\0' && *currentPtr != '/') {
            *normalPtr = *currentPtr;
            currentPtr++;
            normalPtr++;
        }

        if (*currentPtr != '\0') {
            *normalPtr = *currentPtr;
            currentPtr++;
            normalPtr++;
        }
    }
    *normalPtr = '\0';
}
