#pragma once

typedef enum {
    EXCEPTION_OK = 0,
    EXCEPTION_EXIT,
    EXCEPTION_INVALID_JMP_ADDR,
    EXCEPTION_INVALID_RET_ADDR,
    EXCEPTION_DIV_BY_ZERO,
    EXCEPTION_UNKNOWN_INSTRUCTION,
    EXCEPTION_UNKNOWN_STACK_DATA_TYPE,
    EXCEPTION_STACK_OVERFLOW,
    EXCEPTION_STACK_UNDERFLOW,
    EXCEPTION_SUBTRACT_POINTER_AND_DOUBLE,
    EXCEPTION_SUBTRACT_POINTER_AND_INT,
    EXCEPTION_ADD_POINTER_AND_DOUBLE,
    EXCEPTION_ADD_TWO_POINTERS,
    EXCEPTION_MUL_POINTER,
    EXCEPTION_DIV_POINTER,
    EXCEPTION_BITWISE_NON_INT,
    EXCEPTION_MODULO_NON_INT,
    EXCEPTION_INVALID_MEM_ADDR,
    EXCEPTION_INVALID_READ_WRITE_SIZE
} Exception_Type;

void exception_handler(Exception_Type exception);
