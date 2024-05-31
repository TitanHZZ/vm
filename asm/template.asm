%define SYS_EXIT 60
%define STACK_CAP 1024

section .text
    global _start

_start:
    mov rax, SYS_EXIT
    mov rdi, 0 ; exit code
    syscall

sector .data
stack_top: stack

sector .bss
stack: resq STACK_CAP
