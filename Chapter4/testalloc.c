#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>


extern void*
    my_malloc(size_t size);

extern void
    my_free(void *ptr);

extern void
    myalloc_initialize(int fd /* открытый на R/W файловый дескриптор существующего файла */);

extern void
    myalloc_finalize();

int main() {

    int fd = open("swap.superpuper", O_RDWR);
    myalloc_initialize(fd);
   
    void* ptr1 = my_malloc(10);
 printf ("okoko11111\n");
    void* ptr2 = my_malloc(10);
 printf ("okoko2222\n");
    void* ptr3 = my_malloc(10);
printf ("okoko3333\n");

    my_free(ptr1);
printf ("free11111\n");
    ptr1 = my_malloc(5);
    my_free(ptr2);
    ptr2 = my_malloc(7);
printf ("okoko\n");
    my_free(ptr2);
    my_free(ptr3);
    my_free(ptr1);
    myalloc_finalize();
    close (fd);

    return 0;
}
