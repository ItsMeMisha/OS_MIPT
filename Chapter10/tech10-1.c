#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    ALLRIGHT = 0,
    LACK_OF_ARGS = 1,
    SEND_ERROR = 2,
    RECV_ERROR = 3
} Error;

const char dns[] = "8.8.8.8";
const size_t maxDomainSize = 256;
const size_t maxQuerySize = 512;
uint16_t QTYPE = 1;
uint16_t QCLASS = 1;
uint32_t NAME = 0;
uint16_t TYPE = 0;
uint16_t CLASS = 0;
uint16_t RDLENGTH = 0;
uint32_t TTL = 0;

typedef struct {
    uint16_t id;

    unsigned rd :1;
    unsigned tc :1;
    unsigned aa :1;
    unsigned opcode :4;
    unsigned qr :1;

    unsigned rcode :4;
    unsigned cd :1;
    unsigned ad :1;
    unsigned z :1;
    unsigned ra :1;

    uint16_t qCount;
    uint16_t aCount;
    uint16_t authCount;
    uint16_t addCount;

} QueryHeader;

QueryHeader queryHeader = {
    .id = 0xAAAA,
    .rd = 1,
    .tc = 0,
    .aa = 0,
    .opcode = 0,
    .qr = 0,

    .rcode = 0,
    .cd = 0,
    .ad = 0,
    .z = 0,
    .ra = 0,
    .qCount = 0,
    .aCount = 0,
    .authCount = 0,
    .addCount = 0,
};

void createQuery(const char* domain, char* query, size_t* size);
void domainCast(const char* domain, char* dest); 
in_addr_t parseDNSAnswer(const char* answer);

int main(const int argc, const char* argv[])
{
    queryHeader.qCount = htons(1);
    in_addr_t addr = inet_addr(dns);
    in_port_t port = 53;
    port = htons(port);

    struct sockaddr_in src_addr = {
        .sin_family = AF_INET, .sin_port = port, .sin_addr = {addr}};
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    char* query  = (char*) calloc (maxQuerySize, sizeof(char));
    char* answer = (char*) calloc (maxQuerySize, sizeof(char));

    size_t querySize = 0;
    char* domain = (char*) calloc (maxDomainSize, sizeof(char));
    Error error = ALLRIGHT;
    while (scanf("%s", domain) > 0) {
        createQuery(domain, query, &querySize);
        if (sendto(
                sock,
                query,
                querySize,
                0,
                (struct sockaddr*)&src_addr,
                sizeof(src_addr)) < 0) {
            error = SEND_ERROR;
            goto End;
        }

        if (recvfrom(sock, answer, maxQuerySize, 0, NULL, NULL) < 0) {
            error = RECV_ERROR;
            goto End;
        }
printf("RAW:\n");
write(1, answer, maxQuerySize);
        in_addr_t ip = parseDNSAnswer(answer);
        printf("%s\n", inet_ntoa(*(struct in_addr*)&ip));
    }

End:
    free(query);
    free(answer);
    free(domain);
    close(sock);
    return error;
}

void createQuery(const char* domain, char* query, size_t* size) {
    memcpy(query, &queryHeader, sizeof(queryHeader));
printf("ID: %hx\n", ((QueryHeader*)query)->id);
printf("QDCOUNT: %hx\n", ((QueryHeader*)query)->qCount);
printf("ANCOUNT: %hx\n", ((QueryHeader*)query)->aCount);

    query += sizeof(queryHeader);
    *size += sizeof(queryHeader);
    domainCast(domain, query);

    while(*query != '\0') {
        query += *query + 1;
        *size += *query + 1;
    }
    ++query;
    ++(*size);
    
    *(uint16_t*) query = htons(QTYPE);
printf("QTYPE: %hx\n", *(uint16_t*)(query));

    query += sizeof(QTYPE);
    *size += sizeof(QTYPE);
    *(uint16_t*) query = htons(QCLASS);
printf("QCLASS: %hx\n", *(uint16_t*)(query));

    *size += sizeof(QCLASS);
    return;
}

void domainCast(const char* domain, char* dest) {
    char* nextPoint = NULL;
    while(NULL != (nextPoint = strchr(domain, '.'))) {
        *dest = nextPoint - domain;
        memcpy(dest + 1, domain, *dest);
        domain += *dest + 1;
        dest += *dest + 1;
    }

    *dest = strlen(domain);
    memcpy(dest + 1, domain, *dest);
    dest += *dest + 1;
    *dest = '\0';
}

in_addr_t parseDNSAnswer(const char* answer) {
printf("ANSWER\n");
printf("ID: %hx\n", ((QueryHeader*)answer)->id);
printf("QDCOUNT: %hx\n", ((QueryHeader*)answer)->qCount);
printf("ANCOUNT: %hx\n", ((QueryHeader*)answer)->aCount);
    answer += sizeof(queryHeader);
    while(*answer != '\0')
        answer += *answer + 1;
    answer += 1 + sizeof(QTYPE) + sizeof(QCLASS); //go to answer section
    answer += sizeof(NAME) + sizeof(TYPE) + sizeof(CLASS) + sizeof(TTL) + sizeof(RDLENGTH);

    in_addr_t ip = *(in_addr_t*) answer;
    return ip;
}

