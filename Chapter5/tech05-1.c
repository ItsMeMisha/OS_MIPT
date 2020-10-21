#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 2)
        return 1;

    int num = atoi(argv[1]);
    const int LastNum = num;

    int status = 0;
    pid_t pid = fork();
   
    while (num > 0) {
        if (pid == 0) {
            if (num > 1) {
                num--;
                pid = fork();
            } else
                exit(0);
        } else {
            waitpid(-1, &status, 0);

            if (num != LastNum)
                printf("%d ", num);
            else printf("%d\n", num);

            exit(0);
        }
    }
}
