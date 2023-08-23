#pragma once
#include <iostream>
#include <stdint.h>

typedef uint64_t Word;
typedef enum {
    INST_NOP = 0,
    INST_HALT, // debug inst
    INST_EXIT,
    INST_PUSH,
    INST_POP,
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_JMP,
    INST_EQU,
    INST_JMP_IF,
    INST_DUP,
    INST_COUNT // this is not a valid instruction (used to known how many instructions we have)
} Inst_Type;

typedef struct {
    Inst_Type type;
    Word operand;
} Inst;

static const char *inst_type_as_cstr(const Inst_Type& inst) {
    switch (inst) {
    case Inst_Type::INST_NOP:    return "nop";
    case Inst_Type::INST_PUSH:   return "push";
    case Inst_Type::INST_POP:    return "pop";
    case Inst_Type::INST_ADD:    return "add";
    case Inst_Type::INST_SUB:    return "sub";
    case Inst_Type::INST_MUL:    return "mul";
    case Inst_Type::INST_DIV:    return "div";
    case Inst_Type::INST_JMP:    return "jmp";
    case Inst_Type::INST_HALT:   return "halt";
    case Inst_Type::INST_EXIT:   return "exit";
    case Inst_Type::INST_EQU:    return "equ";
    case Inst_Type::INST_JMP_IF: return "jif";
    case Inst_Type::INST_DUP:    return "dup";
    case Inst_Type::INST_COUNT:
    default:
        std::cerr << "ERROR: Could not parse instruction, unknown instruction." << std::endl;
        exit(1);
    }
}

static inline bool inst_requires_operand(const Inst_Type& inst) {
    return inst == INST_PUSH || inst == INST_JMP || inst == INST_JMP_IF || inst == INST_DUP;
}
