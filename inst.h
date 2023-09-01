#pragma once
#include <iostream>
#include <stdint.h>

#include "nan_box.h"

typedef int64_t Word;
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
    INST_DUMP,
    INST_SWAP,
    INST_CALL,
    INST_RET,
    INST_COUNT // this is not a valid instruction (used to known how many instructions we have)
} Inst_Type;

typedef struct {
    Inst_Type type;
    Nan_Box operand;
} Inst;

#define MAKE_INST_NOP()       (Inst) {.type = Inst_Type::INST_NOP, .operand = 0}
#define MAKE_INST_HALT()      (Inst) {.type = Inst_Type::INST_HALT, .operand = 0}
#define MAKE_INST_EXIT()      (Inst) {.type = Inst_Type::INST_EXIT, .operand = 0}
#define MAKE_INST_PUSH(val)   (Inst) {.type = Inst_Type::INST_PUSH, .operand = val}
#define MAKE_INST_POP()       (Inst) {.type = Inst_Type::INST_POP, .operand = 0}
#define MAKE_INST_ADD()       (Inst) {.type = Inst_Type::INST_ADD, .operand = 0}
#define MAKE_INST_SUB()       (Inst) {.type = Inst_Type::INST_SUB, .operand = 0}
#define MAKE_INST_MUL()       (Inst) {.type = Inst_Type::INST_MUL, .operand = 0}
#define MAKE_INST_DIV()       (Inst) {.type = Inst_Type::INST_DIV, .operand = 0}
#define MAKE_INST_JMP(val)    (Inst) {.type = Inst_Type::INST_JMP, .operand = val}
#define MAKE_INST_EQU()       (Inst) {.type = Inst_Type::INST_EQU, .operand = 0}
#define MAKE_INST_JMP_IF(val) (Inst) {.type = Inst_Type::INST_JMP_IF, .operand = val}
#define MAKE_INST_DUP(val)    (Inst) {.type = Inst_Type::INST_DUP, .operand = val}
#define MAKE_INST_DUMP(val)   (Inst) {.type = Inst_Type::INST_DUMP, .operand = 0}
#define MAKE_INST_SWAP(val)   (Inst) {.type = Inst_Type::INST_SWAP, .operand = val}
#define MAKE_INST_CALL(val)   (Inst) {.type = Inst_Type::INST_CALL, .operand = val}

static const char *inst_type_as_cstr(const Inst_Type& inst) {
    switch (inst) {
    case Inst_Type::INST_NOP:      return "nop";
    case Inst_Type::INST_PUSH:     return "push";
    case Inst_Type::INST_POP:      return "pop";
    case Inst_Type::INST_ADD:      return "add";
    case Inst_Type::INST_SUB:      return "sub";
    case Inst_Type::INST_MUL:      return "mul";
    case Inst_Type::INST_DIV:      return "div";
    case Inst_Type::INST_JMP:      return "jmp";
    case Inst_Type::INST_HALT:     return "halt";
    case Inst_Type::INST_EXIT:     return "exit";
    case Inst_Type::INST_EQU:      return "equ";
    case Inst_Type::INST_JMP_IF:   return "jif";
    case Inst_Type::INST_DUP:      return "dup";
    case Inst_Type::INST_DUMP:     return "dump";
    case Inst_Type::INST_SWAP:     return "swap";
    case Inst_Type::INST_CALL:     return "call";
    case Inst_Type::INST_RET:      return "ret";
    case Inst_Type::INST_COUNT:
    default:
        std::cerr << "ERROR: Could not parse instruction, unknown instruction." << std::endl;
        exit(1);
    }
}

static inline bool inst_requires_operand(const Inst_Type& inst) {
    return inst == INST_PUSH || inst == INST_JMP || inst == INST_JMP_IF || inst == INST_DUP || inst == INST_SWAP || inst == INST_CALL;
}

static inline bool inst_operand_might_be_label(const Inst_Type& inst) {
    return inst == INST_JMP || inst == INST_JMP_IF || inst == INST_CALL;
}

static inline bool inst_accepts_fp_operand(const Inst_Type& inst) {
    return inst == INST_PUSH;
}

/*static inline bool inst_operand_cannot_be_negative(const Inst_Type& inst) {
    return inst == INST_JMP || inst == INST_JMP_IF;
}*/
