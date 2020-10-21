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
