#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    SEND_ERROR = 2,
    RECV_ERROR = 3
} Error;

const char localhost[] = "127.0.0.1";

int main(const int argc, const char* argv[])
{

    if (argc < 2)
        return LACK_OF_ARGS;

    in_addr_t addr = inet_addr(localhost);
    in_port_t port = 0;
    sscanf(argv[1], "%hu", &port);
    port = htons(port);

    struct sockaddr_in src_addr = {
        .sin_family = AF_INET, .sin_port = port, .sin_addr = {addr}};
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    int number = 0;
    Error error = ALLRIGHT;
    while (scanf("%d", &number) > 0) {
        if (sendto(
                sock,
                &number,
                sizeof(number),
                0,
                (struct sockaddr*)&src_addr,
                sizeof(src_addr)) < 0) {
            error = SEND_ERROR;
            goto End;
        }

        if (recvfrom(sock, &number, sizeof(number), 0, NULL, NULL) < 0) {
            error = RECV_ERROR;
            goto End;
        }
        printf("%d\n", number);
    }

End:
    close(sock);
    return error;
}
