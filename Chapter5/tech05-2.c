//SOMETHING WRONG WITH SCANF BUFFER!!!
/* На стандартном потоке ввода задается строка текста, которая состоит слова (последовательности непробельных символов), между которыми может быть произвольное количество пробельных символов, включая перевод строки.

Необходимо посчитать количество слов, если известно, что их не больше, чем 255, и вывести это значение на стандартный поток вывода.

Используйте создание новых процессов таким образом, чтобы каждый процесс читал не более одного слова, например, c помощью scanf("%s", ...).

Вывод результата возможен только из того процесса, который запущен самым первым (т.е. из исходной программы).

Итоговая программа должна вернуться с кодом возврата 0.

Размер каждого слова не превышает 4096 байт.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const int MaxStrLen = 4096;

int main()
{
    int status = 0;
    pid_t pid = fork();

    if (pid == 0) {
        char* buffer = (char*)calloc(MaxStrLen, sizeof(char));

        while (1) {
            if (scanf("%s", buffer) > 0) {
                if (pid == -1) {
                    free(buffer);
                    exit(0);
                } else if (pid == 0)
                    pid = fork();

                if (pid > 0) {
                    free(buffer);
                    waitpid(pid, &status, 0);
                    exit(WEXITSTATUS(status) + 1);
                }
            } else {
                free(buffer);
                exit(0);
            }
        }

    } else {
        waitpid(pid, &status, 0);
        printf("%d", WEXITSTATUS(status));
        exit(0);
    }
}
