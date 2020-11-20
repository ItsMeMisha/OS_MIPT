#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>

typedef enum { ALLRIGHT = 0, LACK_OF_ARGS = 1, FAILED_TO_CONNECT = 2 } Error;

int main(const int argc, const char* argv[])
{
    if (argc < 3)
        return LACK_OF_ARGS;

    in_addr_t addr = inet_addr(argv[1]);
    in_port_t port = 0;
    sscanf(argv[2], "%hu", &port);
    port = htons(port);

    struct sockaddr_in connection = {
        .sin_family = AF_INET, .sin_port = port, .sin_addr = {addr}};
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (0 != connect(sock, (struct sockaddr*)&connection, sizeof(connection))) {
        close(sock);
        return FAILED_TO_CONNECT;
    }

    int number = 0;
    while (EOF != scanf("%d", &number)) {
        if (send(sock, &number, sizeof(number), 0) <= 0)
            goto end;

        if (recv(sock, &number, sizeof(number), 0) < sizeof(number))
            goto end;

        printf("%d\n", number);
    }

end:
    shutdown(sock, SHUT_RDWR);
    close(sock);
    return ALLRIGHT;
}
