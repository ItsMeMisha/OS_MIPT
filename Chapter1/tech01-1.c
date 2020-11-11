/* Реализуйте на языке Си программу, которая копирует содержимое потока ввода на поток вывода.

Использование стандартной библиотеки Си запрещено, единственная доступная функция - это syscall(2).

Точка входа в программу - функция _start.

Для использования syscall можно включить в текст программы следующее объявление:

long syscall(long number, ...);
*/
#include <sys/syscall.h>

long syscall(long number, ...);

const int BufferSize = 10 << 10; //Size of buffer is 10KB

void _start() {
    char buffer[BufferSize];
    int numOfCharsRead = BufferSize;

    while (numOfCharsRead == BufferSize) {
        numOfCharsRead = syscall(SYS_read, 0, buffer, BufferSize);
        syscall(SYS_write, 1, buffer, numOfCharsRead);
    }
    syscall(SYS_exit, 0);
}
