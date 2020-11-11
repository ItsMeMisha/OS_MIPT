//BAD VERSION, DOESN'T WORK!!!
/* Необходимо реализовать функциональность функций, аналогичных malloc и free, которые выделяют память в явно заданном swap-файле (файле подкачки), который существует на диске, и имеет некоторый фиксированный размер.

Реализуйте функции:

    extern void*
    my_malloc(size_t size);

    extern void
    my_free(void *ptr);
которые ведут аналогичным образом с malloc и free.
А также функции, которые вызываются один раз: в начале работы программы, и в конце:

    extern void
    myalloc_initialize(int fd * открытый на R/W файловый дескриптор существующего файла *);

    extern void
    myalloc_finalize();
Используйте стратегию наиболее экономичного расхода памяти в файле подкачки.
*/


#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>

#define _FILE_OFFSET_BITS 64

void* my_alloc_buffer = NULL;
off_t my_alloc_buffer_size = 0;

typedef struct {
    void* nextPtr;
    off_t size;
} Block_Header;

void* my_malloc(size_t size) {
    if (size == 0)
        return NULL;

    Block_Header* curBlock = (Block_Header*) my_alloc_buffer;

    if (curBlock->size == 0)
        if (curBlock->nextPtr == NULL) {
            if (size <= my_alloc_buffer_size - sizeof(Block_Header)) {
                curBlock->size = size;
                return curBlock + 1;
        }

        else if (curBlock->nextPtr - (void*)curBlock - 1 >= size){
            curBlock->size = size;
            return curBlock + 1;
        } 
    }

    Block_Header* minFreeBlock = NULL;
    Block_Header* prevBlock = NULL;
    size_t minFreeSuitableSize = 0;

    while (curBlock->nextPtr != NULL) {
        size_t freeBlockSize = (curBlock->nextPtr - (void*)curBlock - 
            2 * sizeof(Block_Header)) - curBlock->size;

        if (minFreeSuitableSize == 0 || freeBlockSize < minFreeSuitableSize)
            if (freeBlockSize >= size) {
                minFreeSuitableSize = freeBlockSize;
                minFreeBlock = (void*) curBlock + 
                    sizeof(Block_Header) + curBlock->size;
                prevBlock = curBlock;
            }
        curBlock = (Block_Header*)curBlock->nextPtr;
    }

    size_t freeBlockSize = my_alloc_buffer_size - ((void*) curBlock - 
        my_alloc_buffer + 2 * sizeof(Block_Header)) - curBlock->size;
    if (minFreeSuitableSize == 0 || freeBlockSize < minFreeSuitableSize)
        if (freeBlockSize >= size) {
            minFreeSuitableSize = freeBlockSize;
            minFreeBlock = (void*) curBlock + 
                sizeof(Block_Header) + curBlock->size;
            prevBlock = curBlock;
        }

    if (minFreeBlock != NULL) {
        minFreeBlock->size = size;
        minFreeBlock->nextPtr = prevBlock->nextPtr;
        prevBlock->nextPtr = (void*) minFreeBlock;
    }
    return minFreeBlock + 1;
}

void my_free(void* ptr) {
    Block_Header* prevBlock = (Block_Header*) my_alloc_buffer;
    ptr -= sizeof(Block_Header);

    if ((void*) prevBlock != ptr) {
        while (prevBlock->nextPtr != ptr && prevBlock != NULL)
            prevBlock = (Block_Header*)prevBlock->nextPtr;
            
        if (prevBlock == NULL)
            return;

        prevBlock->nextPtr =((Block_Header*)ptr)->nextPtr;
    } else 
        ((Block_Header*)ptr)->size = 0;
}

void myalloc_initialize(int fd) {
    struct stat fileInfo;
    fstat(fd, &fileInfo);
    my_alloc_buffer_size = fileInfo.st_size;
    my_alloc_buffer = mmap(NULL, fileInfo.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    ((Block_Header*) my_alloc_buffer)->nextPtr = NULL;
    ((Block_Header*) my_alloc_buffer)->size = 0;
}

void myalloc_finalize() {    
    munmap(my_alloc_buffer, my_alloc_buffer_size);
}
