BITS 64

%define SYS_EXIT 60
%define STACK_CAP 1024

section .text
    global _start

_start:
    mov rax, SYS_EXIT
    mov rdi, 0 ; exit code
    syscall

section .data
stack_top: dq stack

section .bss
stack: resq STACK_CAP
