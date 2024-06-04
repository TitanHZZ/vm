; Nan_Box implementation in assembly
; functions generated by the gcc compiler
BITS 64

extern printf

section .text
    global Nan_Box_box_int, Nan_Box_add, Nan_Box_print, Nan_Box_equ, Nan_Box_get_value

; RDI -> ptr to the result memory location
; RSI -> value to box
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

; RDI -> ptr to the value to print
Nan_Box_print:
    movsd   xmm0, QWORD [rdi]
    ucomisd xmm0, xmm0
    jp      .L59
.L54:
    mov edi, print_double_fmt
    mov eax, 1
    jmp printf
.L59:
    movq rsi, xmm0
    mov  rax, rsi
    shr  rax, 48
    and  eax, 7
    cmp  eax, 1
    je   .L55
    cmp  eax, 2
    je   .L56
    test eax, eax
    je   .L54
    ret
.L56:
    mov rax, 281474976710655
    mov edi, print_ptr_fmt
    and rsi, rax
    xor eax, eax
    jmp printf
.L55:
    mov rax, 281474976710655
    and rsi, rax
    mov rax, rsi
    shr rax, 47
    je  .L58
    mov rax, -281474976710656
    or  rsi, rax
.L58:
    mov edi, print_int_fmt
    xor eax, eax
    jmp printf

; RDI -> ptr to the first value
; RSI -> ptr to the second value
; RAX -> receives the comparison result
Nan_Box_equ:
    movsd   xmm0, QWORD [rdi]
    movsd   xmm1, QWORD [rsi]
    ucomisd xmm0, xmm0
    jp      .L61
    ucomisd xmm1, xmm1
    jp      .L77
.L62:
    ucomisd xmm0, xmm1
    mov     edx, 0
    setnp   al
    cmovne  eax, edx
    ret
.L61:
    movq    rdx, xmm0
    mov     rax, rdx
    shr     rax, 48
    mov     ecx, eax
    and     ecx, 7
    ucomisd xmm1, xmm1
    jp      .L64
    xor     eax, eax
    test    ecx, ecx
    je      .L62
.L60:
    ret
.L77:
    movq rdx, xmm1
    xor  eax, eax
    shr  rdx, 48
    and  edx, 7
    je   .L62
    ret
.L64:
    movq rsi, xmm1
    xor  eax, eax
    mov  rdi, rsi
    shr  rdi, 48
    and  edi, 7
    cmp  edi, ecx
    jne  .L60
    test edi, edi
    je   .L62
    cmp  edi, 1
    je   .L78
    cmp  edi, 2
    jne  .L60
    mov  rcx, 281474976710655
    mov  rax, rsi
    and  rax, rcx
    and  rdx, rcx
.L75:
    cmp  rax, rdx
    sete al
    ret
.L78:
    mov rax, 281474976710655
    and rdx, rax
    mov rax, rdx
    shr rax, 47
    jne .L79
.L68:
    mov rax, 281474976710655
    and rax, rsi
    mov rcx, rax
    shr rcx, 47
    je  .L75
    mov rcx, -281474976710656
    or  rax, rcx
    jmp .L75
.L79:
    mov rax, -281474976710656
    or  rdx, rax
    jmp .L68

; RDI -> ptr to the Nan_Box
; RAX -> receives the number (integer)
Nan_Box_get_value:
    mov     rax, 281474976710655
    mov     rdx, QWORD [rdi]
    movq    xmm0, rdx
    and     rax, rdx
    ucomisd xmm0, xmm0
    jp      .L9
.L7:
    ret
.L9:
    shr rdx, 48
    and edx, 7
    cmp edx, 1
    jne .L7
    mov rcx, rax
    shr rcx, 47
    je  .L7
    mov rdx, -281474976710656
    or  rax, rdx
    ret

section .data
    print_double_fmt: db "%f", 10, 0
    print_int_fmt: db "%ld", 10, 0
    print_ptr_fmt: db "%p", 10, 0
