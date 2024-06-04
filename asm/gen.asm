BITS 64

%define SYS_EXIT 60
%define STACK_CAP 1024

extern Nan_Box_box_int, Nan_Box_add, Nan_Box_print, Nan_Box_equ, Nan_Box_get_value

section .text
    global _start

_start:
    ; push 2
    mov rdi, [stack_top]
    mov rsi, 2
    call Nan_Box_box_int
    add QWORD [stack_top], 8

    ; push 0
    mov rdi, [stack_top]
    mov rsi, 0
    call Nan_Box_box_int
    add QWORD [stack_top], 8

    ; push 1
    mov rdi, [stack_top]
    mov rsi, 1
    call Nan_Box_box_int
    add QWORD [stack_top], 8

    ; print 1
    mov rdi, [stack_top]
    sub rdi, 16
    call Nan_Box_print

    ; print 0
    mov rdi, [stack_top]
    sub rdi, 8
    call Nan_Box_print

label_1:
    ; swap 2
    mov rax, [stack_top]
    mov rbx, [rax-8]
    xchg rbx, [rax-24]
    xchg rbx, [rax-8]

    ; dup 0
    mov rax, [stack_top]
    mov rbx, [rax-8]
    mov [rax], rbx
    add QWORD [stack_top], 8

    ; push 20
    mov rdi, [stack_top]
    mov rsi, 20
    call Nan_Box_box_int
    add QWORD [stack_top], 8

    ; equ
    mov rax, [stack_top]
    lea rdi, [rax-16]
    lea rsi, [rax-8]
    call Nan_Box_equ
    mov rdi, [stack_top]
    sub rdi, 16
    mov rsi, rax
    call Nan_Box_box_int
    sub QWORD [stack_top], 8

    ; jif label_0
    mov rdi, [stack_top]
    sub rdi, 8
    call Nan_Box_get_value
    sub QWORD [stack_top], 8
    cmp rax, 0
    jne label_0

    ; push 1
    mov rdi, [stack_top]
    mov rsi, 1
    call Nan_Box_box_int
    add QWORD [stack_top], 8

    ; add
    mov rax, [stack_top]
    lea rdi, [rax-16]
    lea rsi, [rax-8]
    call Nan_Box_add
    sub QWORD [stack_top], 8

    ; swap 2
    mov rax, [stack_top]
    mov rbx, [rax-8]
    xchg rbx, [rax-24]
    xchg rbx, [rax-8]

    ; dup 0
    mov rax, [stack_top]
    mov rbx, [rax-8]
    mov [rax], rbx
    add QWORD [stack_top], 8

    ; swap 2
    mov rax, [stack_top]
    mov rbx, [rax-8]
    xchg rbx, [rax-24]
    xchg rbx, [rax-8]

    ; add
    mov rax, [stack_top]
    lea rdi, [rax-16]
    lea rsi, [rax-8]
    call Nan_Box_add
    sub QWORD [stack_top], 8

    ; print 0
    mov rdi, [stack_top]
    sub rdi, 8
    call Nan_Box_print

label_0:
    ; exit
    mov rax, SYS_EXIT
    mov rdi, 0
    syscall

section .data
stack_top: dq stack

section .bss
stack: resq STACK_CAP
