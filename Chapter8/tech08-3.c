/* Программа обрабатывать сигнал SIGRTMIN, вместе с которым передается некоторое беззнаковой 32-битное целое число N.

Все остальные сигналы нужно игнорировать.

При получении очередного сигнала нужно уменьшить это число на единицу и отправить его обратно тому процессу, который его послал (используя тот же самый сигнал).

Взаимодействие останавливается при получении значения N==0, после чего нужно корректно завершить работу.

Запрещается использовать signalfd, программа будет компилироваться без поддержки Linux-специфичных расширений.

*/

#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile sig_atomic_t num = 0;
volatile sig_atomic_t sender = 0;

void rtminHandler(int signalNum, siginfo_t* info, void* context)
{
    sender = info->si_pid;
    num = info->si_value.sival_int;
}

int main()
{
    sigset_t blockedSigs;
    sigfillset(&blockedSigs);
    sigdelset(&blockedSigs, SIGRTMIN);
    sigprocmask(SIG_BLOCK, &blockedSigs, NULL);

    struct sigaction sigRtmin = {.sa_sigaction = rtminHandler,
                                 .sa_flags = SA_RESTART | SA_SIGINFO};

    sigaction(SIGRTMIN, &sigRtmin, NULL);

    while (1) {
        pause();
        if (num == 0)
            break;

        union sigval val = {};
        val.sival_int = num - 1;
        sigqueue(sender, SIGRTMIN, val);
    }

    return 0;
}
