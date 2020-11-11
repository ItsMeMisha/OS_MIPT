/*Программе на стандартный поток ввода задается некоторое арифметическое выражение в синтаксисе языка python3.

Необходимо вычислисть это выражение, и вывести результат.

Использовать дополнительные процессы запрещено.
*/


#include <linux/limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
    char arg[ARG_MAX];
    char command[ARG_MAX];
    char* result = fgets(arg, ARG_MAX, stdin);
    if (result == NULL)
        return -1;

    snprintf(command, ARG_MAX, "print(%s)", arg);

    int error = execlp("python3", "python3", "-c", command, NULL);
    if (error == -1)
        return -1;
}
