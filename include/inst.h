#pragma once
#include <iostream>
#include <stdint.h>

#include "nan_box.h"
#include "lexer.h"

typedef int64_t Word;
typedef enum {
    // uncategorized instructions
    INST_NOP = 0, // does nothing
    INST_EXIT,    // terminate the execution

    // stack specific instructions
    INST_PUSH,
    INST_POP,
    INST_SWAP,
    INST_DUP,
    INST_PRINT,

    // type casting instructions
    INST_TD, // convert value to Double
    INST_TI, // convert value to Int
    INST_TP, // convert value to Ptr

    // arithmetic instructions
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_MOD,

    // logical instructions
    INST_AND,
    INST_OR,
    INST_XOR,
    INST_NOT,

    // native functions related instructions
    INST_NATIVE,

    // bitwise instructions
    INST_SHL, // logical shift
    INST_SHR, // logical shift
    INST_SAR, // arithmetic shift

    // control flow instructions
    INST_JMP,
    INST_EQU,
    INST_JMP_IF,
    INST_CALL,
    INST_RET,

    // memory related instructions
    INST_READ,
    INST_WRITE,

    // debug/testing instructions
    INST_DUMP_STACK,
    INST_DUMP_MEMORY,
    INST_HALT, // debug inst (stops the execution)
    INST_COUNT // this is not a valid instruction (used to known how many instructions we have)
} Inst_Type;

typedef struct {
    Inst_Type type;
    Nan_Box operand;
} Inst;

// this is used to remove the warnings about the function not being used
#if defined(__GNUC__) || defined(__clang__)
    #define USED_FUNCTION __attribute__((used))
#elif defined(_MSC_VER)
    #define USED_FUNCTION __pragma(warning(suppress: 4505))
#else
    #define USED_FUNCTION
#endif

static const char *inst_type_as_cstr(const Inst_Type& inst) USED_FUNCTION;
static const char *inst_type_as_cstr(const Inst_Type& inst) {
    switch (inst) {
    case Inst_Type::INST_NOP:         return "nop";
    case Inst_Type::INST_PUSH:        return "push";
    case Inst_Type::INST_POP:         return "pop";
    case Inst_Type::INST_ADD:         return "add";
    case Inst_Type::INST_SUB:         return "sub";
    case Inst_Type::INST_MUL:         return "mul";
    case Inst_Type::INST_DIV:         return "div";
    case Inst_Type::INST_MOD:         return "mod";
    case Inst_Type::INST_JMP:         return "jmp";
    case Inst_Type::INST_HALT:        return "halt";
    case Inst_Type::INST_EXIT:        return "exit";
    case Inst_Type::INST_EQU:         return "equ";
    case Inst_Type::INST_JMP_IF:      return "jif";
    case Inst_Type::INST_DUP:         return "dup";
    case Inst_Type::INST_DUMP_STACK:  return "dump_stack";
    case Inst_Type::INST_DUMP_MEMORY: return "dump_memory";
    case Inst_Type::INST_SWAP:        return "swap";
    case Inst_Type::INST_CALL:        return "call";
    case Inst_Type::INST_RET:         return "ret";
    case Inst_Type::INST_NATIVE:      return "native";
    case Inst_Type::INST_PRINT:       return "print";
    case Inst_Type::INST_SHL:         return "shl";
    case Inst_Type::INST_SHR:         return "shr";
    case Inst_Type::INST_SAR:         return "sar";
    case Inst_Type::INST_AND:         return "and";
    case Inst_Type::INST_OR:          return "or";
    case Inst_Type::INST_XOR:         return "xor";
    case Inst_Type::INST_NOT:         return "not";
    case Inst_Type::INST_READ:        return "read";
    case Inst_Type::INST_WRITE:       return "write";
    case Inst_Type::INST_TD:          return "td";
    case Inst_Type::INST_TI:          return "ti";
    case Inst_Type::INST_TP:          return "tp";
    case Inst_Type::INST_COUNT:
    default:
        std::cerr << "ERROR: Could not parse instruction, unknown instruction." << std::endl;
        exit(1);
    }
}

static bool str_is_inst(const std::string &str) USED_FUNCTION;
static bool str_is_inst(const std::string &str) {
    for (int inst_to_check = 0; inst_to_check < Inst_Type::INST_COUNT; inst_to_check++) {
        // compare the strings
        if (!str.compare(inst_type_as_cstr((Inst_Type)inst_to_check))) {
            return true;
        }
    }

    return false;
}

static Inst_Type str_as_inst(const std::string &str) USED_FUNCTION;
static Inst_Type str_as_inst(const std::string &str) {
    for (int inst_to_check = 0; inst_to_check < Inst_Type::INST_COUNT; inst_to_check++) {
        // compare the strings
        if (!str.compare(inst_type_as_cstr((Inst_Type)inst_to_check))) {
            return (Inst_Type) inst_to_check;
        }
    }

    // should never happen
    return INST_NOP;
}

// TODO: explain this array
constexpr static Token_Type inst_acc_tk[][3] = {
    [INST_NOP]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_EXIT]        = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_PUSH]        = {INTEGER, FP,      KEYWORD},
    [INST_POP]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_SWAP]        = {INTEGER, UNKNOWN, UNKNOWN},
    [INST_DUP]         = {INTEGER, UNKNOWN, UNKNOWN},
    [INST_PRINT]       = {INTEGER, UNKNOWN, UNKNOWN},
    [INST_TD]          = {INTEGER, UNKNOWN, UNKNOWN},
    [INST_TI]          = {INTEGER, UNKNOWN, UNKNOWN},
    [INST_TP]          = {INTEGER, UNKNOWN, UNKNOWN},
    [INST_ADD]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_SUB]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_MUL]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_DIV]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_MOD]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_AND]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_OR]          = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_XOR]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_NOT]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_NATIVE]      = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_SHL]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_SHR]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_SAR]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_JMP]         = {INTEGER, KEYWORD, UNKNOWN},
    [INST_EQU]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_JMP_IF]      = {INTEGER, KEYWORD, UNKNOWN},
    [INST_CALL]        = {INTEGER, KEYWORD, UNKNOWN},
    [INST_RET]         = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_READ]        = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_WRITE]       = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_DUMP_STACK]  = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_DUMP_MEMORY] = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_HALT]        = {UNKNOWN, UNKNOWN, UNKNOWN},
    [INST_COUNT]       = {UNKNOWN, UNKNOWN, UNKNOWN}
};

static inline bool inst_requires_operand(const Inst_Type inst) {
    return inst_acc_tk[inst][0] != Token_Type::UNKNOWN;
}

/*static inline bool inst_requires_operand(const Inst_Type& inst) {
    return inst == INST_PUSH || inst == INST_JMP    || inst == INST_JMP_IF || inst == INST_DUP  || inst == INST_SWAP  ||
           inst == INST_CALL || inst == INST_NATIVE || inst == INST_PRINT  || inst == INST_READ || inst == INST_WRITE ||
           inst == INST_TD   || inst == INST_TI     || inst == INST_TP;
}*/
static inline bool inst_operand_might_be_label(const Inst_Type& inst) {
    return inst == INST_JMP || inst == INST_JMP_IF || inst == INST_CALL;
}
static inline bool inst_operand_might_be_function(const Inst_Type& inst) {
    return inst == INST_NATIVE;
}
static inline bool inst_accepts_fp_operand(const Inst_Type& inst) {
    return inst == INST_PUSH;
}
/*static inline bool inst_operand_cannot_be_negative(const Inst_Type& inst) {
    return inst == INST_JMP || inst == INST_JMP_IF;
}
#define MAKE_INST_NOP()         (Inst) {.type = Inst_Type::INST_NOP, .operand = 0}
#define MAKE_INST_HALT()        (Inst) {.type = Inst_Type::INST_HALT, .operand = 0}
#define MAKE_INST_EXIT()        (Inst) {.type = Inst_Type::INST_EXIT, .operand = 0}
#define MAKE_INST_PUSH(val)     (Inst) {.type = Inst_Type::INST_PUSH, .operand = val}
#define MAKE_INST_POP()         (Inst) {.type = Inst_Type::INST_POP, .operand = 0}
#define MAKE_INST_ADD()         (Inst) {.type = Inst_Type::INST_ADD, .operand = 0}
#define MAKE_INST_SUB()         (Inst) {.type = Inst_Type::INST_SUB, .operand = 0}
#define MAKE_INST_MUL()         (Inst) {.type = Inst_Type::INST_MUL, .operand = 0}
#define MAKE_INST_DIV()         (Inst) {.type = Inst_Type::INST_DIV, .operand = 0}
#define MAKE_INST_JMP(val)      (Inst) {.type = Inst_Type::INST_JMP, .operand = val}
#define MAKE_INST_EQU()         (Inst) {.type = Inst_Type::INST_EQU, .operand = 0}
#define MAKE_INST_JMP_IF(val)   (Inst) {.type = Inst_Type::INST_JMP_IF, .operand = val}
#define MAKE_INST_DUP(val)      (Inst) {.type = Inst_Type::INST_DUP, .operand = val}
#define MAKE_INST_DUMP_STACK()  (Inst) {.type = Inst_Type::INST_DUMP_STACK, .operand = 0}
#define MAKE_INST_DUMP_MEMORY() (Inst) {.type = Inst_Type::INST_DUMP_MEMORY, .operand = 0}
#define MAKE_INST_SWAP(val)     (Inst) {.type = Inst_Type::INST_SWAP, .operand = val}
#define MAKE_INST_CALL(val)     (Inst) {.type = Inst_Type::INST_CALL, .operand = val}
#define MAKE_INST_RET()         (Inst) {.type = Inst_Type::INST_RET, .operand = 0}
#define MAKE_INST_NATIVE(val)   (Inst) {.type = Inst_Type::INST_NATIVE, .operand = val}
#define MAKE_INST_PRINT(val)    (Inst) {.type = Inst_Type::INST_PRINT, .operand = val}
#define MAKE_INST_SHL()         (Inst) {.type = Inst_Type::INST_SHL, .operand = 0}
#define MAKE_INST_SHR()         (Inst) {.type = Inst_Type::INST_SHR, .operand = 0}
#define MAKE_INST_AND()         (Inst) {.type = Inst_Type::INST_AND, .operand = 0}
#define MAKE_INST_OR()          (Inst) {.type = Inst_Type::INST_OR, .operand = 0}
#define MAKE_INST_XOR()         (Inst) {.type = Inst_Type::INST_XOR, .operand = 0}
#define MAKE_INST_NOT()         (Inst) {.type = Inst_Type::INST_NOT, .operand = 0}
#define MAKE_INST_READ8()       (Inst) {.type = Inst_Type::INST_READ8, .operand = 0}
#define MAKE_INST_READ16()      (Inst) {.type = Inst_Type::INST_READ16, .operand = 0}
#define MAKE_INST_READ32()      (Inst) {.type = Inst_Type::INST_READ32, .operand = 0}
#define MAKE_INST_READ64()      (Inst) {.type = Inst_Type::INST_READ64, .operand = 0}
#define MAKE_INST_WRITE8()      (Inst) {.type = Inst_Type::INST_WRITE8, .operand = 0}
#define MAKE_INST_WRITE16()     (Inst) {.type = Inst_Type::INST_WRITE16, .operand = 0}
#define MAKE_INST_WRITE32()     (Inst) {.type = Inst_Type::INST_WRITE32, .operand = 0}
#define MAKE_INST_WRITE64()     (Inst) {.type = Inst_Type::INST_WRITE64, .operand = 0}*/
