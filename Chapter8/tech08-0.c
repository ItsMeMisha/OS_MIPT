/* Программа при запуске сообщает на стандартный поток вывода свой PID, выталкивает буфер вывода с помощью fflush, после чего начинает обрабатывать поступающие сигналы.

При поступлении сигнала SIGTERM необходимо вывести на стандартный поток вывода целое число: количество ранее поступивших сигналов SIGINT и завершить свою работу.

Семантика повединия сигналов (Sys-V или BSD) считается не определенной.

*/

#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile sig_atomic_t counter = 0;
volatile sig_atomic_t endProg = 0;

void termHandler(int signalNum)
{
    endProg = 1;
}

void intHandler(int signalNum)
{
    ++counter;
}

int main()
{
    struct sigaction sigTerm = {.sa_handler = termHandler,
                                .sa_flags = SA_RESTART};
    struct sigaction sigInt = {.sa_handler = intHandler,
                               .sa_flags = SA_RESTART};
    sigaction(SIGTERM, &sigTerm, NULL);
    sigaction(SIGINT, &sigInt, NULL);

    printf("%d\n", getpid());
    fflush(stdout);

    while (endProg == 0)
        ;

    printf("%d\n", counter);
    fflush(stdout);

    return 0;
}
