; Nan_Box implementation in assembly
BITS 64

section .text
    global Nan_Box_box_int, Nan_Box_add

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
    ret

; RDI -> ptr to the first Nan_Box (receives the new value)
; RSI -> ptr to the second Nan_Box
Nan_Box_add:
    movsd   xmm0, QWORD [rdi]
    movsd   xmm1, QWORD [rsi]
    mov     rax, rdi
    ucomisd xmm0, xmm0
    jp      .L13
    ucomisd xmm1, xmm1
    jp      .L45
.L44:
    addsd xmm0, xmm1
    movsd QWORD [rax], xmm0
    ret
.L13:
    movq    rdi, xmm0
    mov     rdx, rdi
    shr     rdx, 48
    and     edx, 7
    ucomisd xmm1, xmm1
    jp      .L16
    test    edx, edx
    je      .L44
    cmp     edx, 1
    je      .L42
.L17:
    ret
.L42:
    mov rdx, 281474976710655
    and rdi, rdx
    mov rdx, rdi
    shr rdx, 47
    cmp rdx, 1
    je  .L46
.L25:
    pxor     xmm0, xmm0
    cvtsi2sd xmm0, rdi
    jmp      .L44
.L45:
    movq rcx, xmm1
    shr  rcx, 48
    and  ecx, 7
.L19:
    test    ecx, ecx
    je      .L44
    cmp     ecx, 1
    jne     .L17
    mov     rcx, 281474976710655
    mov     rdx, QWORD [rsi]
    movq    xmm1, rdx
    and     rcx, rdx
    ucomisd xmm1, xmm1
    jp      .L47
.L22:
    pxor     xmm1, xmm1
    cvtsi2sd xmm1, rcx
    addsd    xmm1, xmm0
    movsd    QWORD  [rax], xmm1
    ret
.L16:
    movq r8, xmm1
    mov  rcx, r8
    shr  rcx, 48
    and  ecx, 7
    test edx, edx
    je   .L19
    cmp  edx, 1
    je   .L48
    cmp  edx, 2
    jne  .L17
    cmp  ecx, 1
    jne  .L17
    mov  rdx, 281474976710655
    and  r8, rdx
    and  rdi, rdx
    mov  rcx, r8
    shr  rcx, 47
    jne  .L49
.L31:
    mov rdx, 281474976710655
    add rdi, r8
    and rdi, rdx
    mov rdx, 9221683186994511872
    or  rdi, rdx
    mov QWORD [rax], rdi
    ret
.L48:
    test ecx, ecx
    je   .L42
    cmp  ecx, 1
    je   .L50
    cmp  ecx, 2
    jne  .L17
    mov  rdx, 281474976710655
    and  rdi, rdx
    and  r8, rdx
    mov  rcx, rdi
    shr  rcx, 47
    jne  .L51
.L30:
    mov  rcx, 281474976710655
    lea rdx, [r8+rdi]
    and rdx, rcx
    mov rcx, 9221683186994511872
    or  rdx, rcx
    mov QWORD [rax], rdx
    ret
.L50:
    mov rdx, 281474976710655
    and rdi, rdx
    mov rcx, rdi
    shr rcx, 47
    jne .L27
.L43:
    and r8, rdx
    mov rdx, r8
    shr rdx, 47
    cmp rdx, 1
    je  .L52
.L29:
    mov rcx, 281474976710655
    lea rdx, [r8+rdi]
    and rdx, rcx
    mov rcx, 9221401712017801216
    or  rdx, rcx
    mov QWORD [rax], rdx
    ret
.L47:
    mov rdx, rcx
    shr rdx, 47
    je  .L22
    mov rdx, -281474976710656
    or  rcx, rdx
    jmp .L22
.L46:
    mov rdx, -281474976710656
    or  rdi, rdx
    jmp .L25
.L49:
    mov edx, 65535
    sal rdx, 48
    or  r8, rdx
    jmp .L31
.L52:
    mov edx, 65535
    sal rdx, 48
    or  r8, rdx
    jmp .L29
.L27:
    mov ecx, 65535
    sal rcx, 48
    or  rdi, rcx
    jmp .L43
.L51:
    mov edx, 65535
    sal rdx, 48
    or  rdi, rdx
    jmp .L30
