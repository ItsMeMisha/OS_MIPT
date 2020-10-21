#include <sys/syscall.h>

long syscall(long number, ...);

void _start() {
  const char message[] = "Hello, World!";

  syscall(SYS_write, 1, message, sizeof(message) - 1);
  syscall(SYS_exit, 0);
}
