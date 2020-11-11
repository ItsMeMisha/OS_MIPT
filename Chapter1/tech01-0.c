/* Реализуйте на языке Си программу, которая выводит "Hello, World!".

Использование стандартной библиотеки Си запрещено, единственная доступная функция - это syscall(2).

Точка входа в программу - функция _start.

Для использования syscall можно включить в текст программы следующее объявление:

long syscall(long number, ...);
*/


#include <sys/syscall.h>

long syscall(long number, ...);

void _start() {
  const char message[] = "Hello, World!";

  syscall(SYS_write, 1, message, sizeof(message) - 1);
  syscall(SYS_exit, 0);
}
