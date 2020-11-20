#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef enum {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    FAILED_TO_CONNECT = 2,
    BAD_LISTEN = 3,
    BAD_ACCEPT = 4,
    MAP_ERROR = 5,
    UNMAP_ERROR = 6,
    BAD_REQUEST = 7
} Error;

const char httpOK[] = "HTTP/1.1 200 OK\r\n";
const char httpNotFound[] = "HTTP/1.1 404 Not Found\r\n";
const char httpForbidden[] = "HTTP/1.1 403 Forbidden\r\n";
const char contentLengthFormat[] = "Content-Length: %ld\r\n\r\n";

const size_t bufSize = 4096;
const mode_t Readable = S_IRUSR | S_IRGRP | S_IROTH;
const mode_t Executable = S_IXUSR | S_IXGRP | S_IXOTH;
int sock = 0;

volatile sig_atomic_t toExit = 0;
volatile sig_atomic_t waitConnection = 0;

void launch(const char* fileName, int newOutput)
{
    pid_t pid = fork();
    if (pid == 0) {
        dup2(newOutput, 1);
        close(newOutput);
        execlp(fileName, fileName, NULL);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
    }
}

void sigHandler(int signalNum)
{
    toExit = 1;
    if (waitConnection) {
        shutdown(sock, SHUT_RDWR);
        close(sock);
        _exit(0);
    }
}

int main(const int argc, const char* argv[])
{
    if (argc < 3)
        return LACK_OF_ARGS;

    Error error = ALLRIGHT;

    struct sigaction sigExit = {.sa_handler = sigHandler,
                                .sa_flags = SA_RESTART};
    sigaction(SIGTERM, &sigExit, NULL);
    sigaction(SIGINT, &sigExit, NULL);

    in_addr_t addr = inet_addr("127.0.0.1");
    in_port_t port = 0;
    sscanf(argv[1], "%hu", &port);
    port = htons(port);

    struct sockaddr_in connection = {
        .sin_family = AF_INET, .sin_port = port, .sin_addr = {addr}};
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (0 != bind(sock, (struct sockaddr*)&connection, sizeof(connection))) {
        error = FAILED_TO_CONNECT;
        goto end;
    }

    if (-1 == listen(sock, SOMAXCONN)) {
        error = BAD_LISTEN;
        goto end;
    }

    while (toExit == 0) {
        waitConnection = 1;
        int clientSocket = accept(sock, NULL, NULL);
        if (-1 == clientSocket) {
            close(sock);
            return BAD_ACCEPT;
        }
        waitConnection = 0;

        char* path = (char*)calloc(sizeof(char), PATH_MAX);
        strncpy(path, argv[2], PATH_MAX);
        size_t pathLength = strlen(path);
        if (pathLength < PATH_MAX && path[pathLength - 1] != '/') {
            path[pathLength] = '/';
            ++pathLength;
        }
        path[pathLength] = '\0';

        char* bufInput = (char*)calloc(sizeof(char), bufSize);
        int hasRead = recv(clientSocket, bufInput, bufSize, 0);
        if (hasRead < sizeof("GET   HTTP/1.1")) {
            error = BAD_REQUEST;
            free(bufInput);
            free(path);
            goto connectionEnd;
        }

        char* fileName = bufInput + 4;
        char* fileNameEnd = strstr(bufInput + 4, " HTTP/1.1");
        size_t nameLength = fileNameEnd - fileName;
        strncat(path, fileName, nameLength);

        while (recv(clientSocket, bufInput, bufSize, MSG_DONTWAIT) > 0)
            if (strstr(bufInput, "\r\n\r\n") != NULL)
                break;
        free(bufInput);

        struct stat fileInfo;
        if (-1 == lstat(path, &fileInfo)) {
            send(clientSocket, httpNotFound, sizeof(httpNotFound) - 1, 0);
            free(path);
            goto connectionEnd;
        }

        if (!(fileInfo.st_mode & Readable)) {
            send(clientSocket, httpForbidden, sizeof(httpForbidden) - 1, 0);
            free(path);
            goto connectionEnd;
        }

        int file = open(path, O_RDONLY);
        free(path);
        send(clientSocket, httpOK, sizeof(httpOK) - 1, 0);

        if (fileInfo.st_mode & Executable) {
            launch(path, clientSocket);
            goto connectionEnd;
        }

        dprintf(clientSocket, contentLengthFormat, fileInfo.st_size);
        char* buffer =
            mmap(NULL, fileInfo.st_size, PROT_READ, MAP_SHARED, file, 0);
        if (buffer == MAP_FAILED) {
            close(file);
            error = MAP_ERROR;
            goto connectionEnd;
        }

        send(clientSocket, buffer, fileInfo.st_size, 0);
        int munmapResult = munmap(buffer, fileInfo.st_size);
        close(file);
        if (-1 == munmapResult) {
            error = UNMAP_ERROR;
            goto connectionEnd;
        }

    connectionEnd:
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        if (error != ALLRIGHT)
            goto end;
    }

end:
    shutdown(sock, SHUT_RDWR);
    close(sock);
    return error;
}
