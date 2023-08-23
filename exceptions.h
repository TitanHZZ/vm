#pragma once

typedef enum {
    EXCEPTION_OK = 0,
    EXCEPTION_EXIT,
    EXCEPTION_UNKNOWN_INSTRUCTION,
    EXCEPTION_STACK_OVERFLOW,
    EXCEPTION_STACK_UNDERFLOW,
    EXCEPTION_INVALID_JMP_ADDR,
    EXCEPTION_DIV_BY_ZERO
} Exception_Type;

void exception_handler(Exception_Type exception);
