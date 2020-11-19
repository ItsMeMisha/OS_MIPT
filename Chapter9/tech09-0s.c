#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>

typedef enum {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    FAILED_TO_CONNECT = 2
} Error;

int main(const int argc, const char* argv[]) {
    if (argc < 3)
        return LACK_OF_ARGS;
printf("%s\n", argv[1]);
    in_addr_t addr = inet_addr(argv[1]);
    addr = htonl(addr);
printf ("%u\n", addr);
    in_port_t port = atoi(argv[2]);
//    port = htons(port);
printf ("%u\n", port);

    struct sockaddr_in connection = {.sin_family = AF_INET, .sin_port = port, .sin_addr = {addr}};
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (0 != bind(sock, (struct sockaddr*) &connection, sizeof(connection))) {
        close(sock);
perror("ahh");
        return FAILED_TO_CONNECT;
    }

    listen(sock, SOMAXCONN);
    accept(sock, NULL, NULL);
    int number = 0;
    do {
        scanf("%d", &number);
        if (recv(sock, &number, sizeof(number), 0) < sizeof(number))
            goto end;
        number += 1;
        if (send(sock, &number, sizeof(number), 0) < 0)
            goto end;

//        printf("%d\n", number);
    }   while(1);

end:

    close(sock);
    return ALLRIGHT;
}
