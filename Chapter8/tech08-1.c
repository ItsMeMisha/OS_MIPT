/* Программа при запуске сообщает на стандартный поток вывода свой PID, после чего читает со стандартного потока вывода целое число - начальное значение, которое затем будет изменяться.

При поступлении сигнала SIGUSR1 увеличить текущее значение на 1 и вывести его на стандартный поток вывода.

При поступлении сигнала SIGUSR2 - умножить текущее значение на -1 и вывести его на стандартный поток вывода.

Семантика повединия сигналов (Sys-V или BSD) считается не определенной.

Не забывайте выталкивать буфер вывода.
*/

#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile sig_atomic_t num = 0;
volatile sig_atomic_t changedValue = 0;

void usr1Handler(int signalNum)
{
    changedValue = 1;
}

void usr2Handler(int signalNum)
{
    changedValue = 2;
}

void termHandler(int signalNum)
{
    _exit(0);
}

int main()
{
    struct sigaction sigUsr1 = {.sa_handler = usr1Handler,
                                .sa_flags = SA_RESTART};
    struct sigaction sigUsr2 = {.sa_handler = usr2Handler,
                                .sa_flags = SA_RESTART};
    struct sigaction sigTerm = {.sa_handler = termHandler,
                                .sa_flags = SA_RESTART};

    sigaction(SIGUSR1, &sigUsr1, NULL);
    sigaction(SIGUSR2, &sigUsr2, NULL);
    sigaction(SIGTERM, &sigTerm, NULL);

    printf("%d\n", getpid());
    fflush(stdout);
    scanf("%d", &num);

    while (1) {
        pause();
        if (changedValue == 1)
            ++num;
        else if (changedValue == 2)
            num *= -1;

        if (changedValue) {
            printf("%d\n", num);
            fflush(stdout);
            changedValue = 0;
        }
    }
    return 0;
}
