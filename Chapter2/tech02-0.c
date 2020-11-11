/*Программе в аргументах командной строки передаются три имени файла. Первый аргумент - входной файл, два остальных - выходные.

Реализуйте программу, которая читает символы из первого файла, во второй файл записывает только цифры, а в третий - всё остальное.

Разрешается использовать только низкоуровневый ввод-вывод POSIX.

Если входной файл не существует, то нужно завершить работу с кодом 1.

Если не возможно создать один из выходных файлов, то завершить работу с кодом 2.

При возникновении других ошибок ввода-вывода - завершить работу с кодом 3.

*/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>

typedef enum {
    ALLRIGHT = 0,
    FILE_DO_NOT_EXIST = 1,
    UNABLE_CREATE_FILE = 2,
    OTHER_ERROR = 3
} Error;

Error error = ALLRIGHT;
const uint32_t bufferSize = 4 << 10 << 10; //4 kb
const mode_t rightsForNewFiles = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

int main(int argc, char* argv[]) {
    if (argc < 4)
        return OTHER_ERROR;

    int input = open(argv[1], O_RDONLY);
    if (input == -1)
        return FILE_DO_NOT_EXIST;

    int outputDigits = open(argv[2], O_WRONLY | O_CREAT, rightsForNewFiles);
    int outputNotDigits = open(argv[3], O_WRONLY | O_CREAT, rightsForNewFiles);
    if (outputDigits == -1 || outputNotDigits == -1)
        return UNABLE_CREATE_FILE;

    char buffer[bufferSize];

    int charsRead = 1;
    int writeReturnValue = 0;
    while (charsRead > 0) {
        charsRead = read(input, buffer, bufferSize);
        if (charsRead == -1) {
            error = OTHER_ERROR;
            goto finally;
        }

        for (int i = 0; i < charsRead; ++i) {
            if (isdigit(buffer[i]))
                writeReturnValue = write(outputDigits, buffer + i, 1);
            else
                writeReturnValue = write(outputNotDigits, buffer + i, 1);
        }

        if (writeReturnValue == -1) {
            error = OTHER_ERROR;
            goto finally;
        }
    }

finally:
    close(outputNotDigits);
    close(outputDigits);
    close(input);
    
    return error;
}
