#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

const int MaxStrLen = 4096;

int main() {
    int status = 0;
    pid_t pid = fork();

    if (pid == 0) {
        char* buffer = (char*)calloc(MaxStrLen, sizeof(char));

        while(1) {
            if (pid == 0) {

                if (scanf("%s", buffer) > 0) {
                    pid = fork();
                }
                else {
                    free(buffer);
                    exit(0);
                }
            } else {
                free(buffer);
                waitpid(pid, &status, 0);
                exit(WEXITSTATUS(status) + 1);
            }
        }

    } else {
        waitpid(pid, &status, 0);
        printf("%d", WEXITSTATUS(status));
        exit(0);
    }
}
