/* Программе в качестве аргументов передаются N имен текстовых файлов.

Программа должна обрабатывать множество сигналов от SIGRTMIN до SIGRTMAX, причем номер сигнала в диапазоне от SIGRTMIN+1 определяет порядковый номер файла из аргументов:

x = signo - SIGRTMIN; // SIGRTMIN <= signo <= SIGRTMAX
                      // 1 <= x <= SIGRTMAX-SIGRTMIN
При получении очередного сигнала необходимо прочитать одну строку из определенного файла и вывести её на стандартный поток вывода.

При получении сигнала с номером SIGRTMIN, т.е. при номере аргумента, равным 0, - корректно завершить свою работу с кодом 0.

Все остальные сигналы нужно игнорировать.

Если для вывода используются высокоуровневые функции стандартной библиотеки Си, то необходимо выталкивать буфер обмена после вывода каждой строки.
*/

#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile sig_atomic_t signo = 0;
volatile sig_atomic_t endProg = 0;
const size_t MaxLength = 4096;

void commonHandler(int signalNum)
{
    signo = signalNum;
}

void rtminHandler(int signalNum)
{
    endProg = 1;
}

int main(const int argc, const char* argv[])
{
    char str[MaxLength];
    FILE* files[argc];
    for (int i = 1; i < argc; ++i)
        files[i - 1] = fopen(argv[i], "r");

    struct sigaction sigCommon = {.sa_handler = commonHandler,
                                  .sa_flags = SA_RESTART};
    struct sigaction sigRTMIN = {.sa_handler = rtminHandler,
                                 .sa_flags = SA_RESTART};

    sigset_t blockedSigs;
    sigfillset(&blockedSigs);

    for (int signo = SIGRTMAX; signo > SIGRTMIN; --signo) {
        sigaction(signo, &sigCommon, NULL);
        sigdelset(&blockedSigs, signo);
    }

    sigaction(SIGRTMIN, &sigRTMIN, NULL);
    sigdelset(&blockedSigs, SIGRTMIN);

    sigprocmask(SIG_BLOCK, &blockedSigs, NULL);

    while (1) {
        pause();
        if (endProg)
            break;

        if (signo != 0) {
            fgets(str, MaxLength, files[signo - SIGRTMIN - 1]);
            fprintf(stdout, "%s", str);
            fflush(stdout);
            signo = 0;
        }
    }

    for (int i = 1; i < argc; ++i)
        fclose(files[i - 1]);
    return 0;
}
