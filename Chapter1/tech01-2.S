/*Реализуйте на языке ассемблера x86_64 программу, которая копирует содержимое со стандартного потока ввода на стандартный поток вывода.

Использование стандартной библиотеки Си запрещено.

Точка входа в программу - функция _start.

*/

#include <sys/syscall.h>
.intel_syntax noprefix
.text
.global _start

_start:
    BufferSize = 10 * 1024
    
    //Get address
    mov rax, SYS_brk
    xor rdi, rdi
    syscall

    mov rsi, rax

    //Make buffer
    mov rdi, rsi
    add rdi, BufferSize
    mov rax, SYS_brk
    syscall

    //loop to read and write
    loop:
    //Read
    mov rax, SYS_read
    mov rdi, 0
    mov rdx, BufferSize    
    syscall

    //Write
    mov rdx, rax
    mov rax, SYS_write
    mov rdi, 1
    syscall

    cmp rdx, BufferSize
    je loop

    //Exit
    mov rax, SYS_exit
    xor rdi, rdi
    syscall
    
