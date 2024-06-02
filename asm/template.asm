BITS 64

%define SYS_EXIT 60
%define STACK_CAP 1024

extern Nan_Box_box_int

section .text
    global _start

_start:
    mov rdi, [stack_top]
    mov rsi, 10
    call Nan_Box_box_int
    add QWORD [stack_top], 8

    mov rax, SYS_EXIT
    mov rdi, 0 ; exit code
    syscall

section .data
stack_top: dq stack

section .bss
stack: resq STACK_CAP
