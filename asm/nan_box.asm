; Nan_Box implementation in assembly
BITS 64

section .text
    global Nan_Box_box_int

; RDI -> ptr to the result memory location
; RSI -> type to set
Nan_Box_set_type:
    mov rax, 9221120237041090560
    sal rsi, 48
    or  rsi, rax
    mov QWORD [rdi], rsi
    ret

; RDI -> ptr to the result memory location
; RSI -> value to set
Nan_Box_set_value:
    mov rax, 281474976710655
    and rsi, rax
    or  QWORD [rdi], rsi
    ret

; RDI -> ptr to the result memory location
; RSI -> Value to box
Nan_Box_box_int:
    mov rax, 281474976710655
    and rsi, rax
    mov rax, 9221401712017801216
    or  rsi, rax
    mov QWORD [rdi], rsi
    ; movq    xmm0, rsi
    ret
