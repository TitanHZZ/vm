BITS 64

%define SYS_EXIT 60
%define STACK_CAP 1024

extern Nan_Box_box_int

section .text
    global _start

_start:
    ; PUSH
    mov rdi, [stack_top]
    mov rsi, 10
    call Nan_Box_box_int
    add QWORD [stack_top], 8

    ; PUSH
    mov rdi, [stack_top]
    mov rsi, 20
    call Nan_Box_box_int
    add QWORD [stack_top], 8

    ; PUSH
    mov rdi, [stack_top]
    mov rsi, 30
    call Nan_Box_box_int
    add QWORD [stack_top], 8

    ; EXIT
    mov rax, SYS_EXIT
    mov rdi, 0
    syscall

section .data
stack_top: dq stack

section .bss
stack: resq STACK_CAP
