#pragma once

#include "lexer.h"

typedef enum {
    ALIAS = 0,
    INCLUDE,
    RES,
    STR
} Directive_Type;

constexpr static Token_Type dir_acc_tk[][2][2] = {
    [ALIAS]   = {
        {KEYWORD, UNKNOWN},
        {INTEGER, FP}
    },
    [INCLUDE] = {
        {UNKNOWN, UNKNOWN},
        {UNKNOWN, UNKNOWN}
    },
    [RES]     = {
        {UNKNOWN, UNKNOWN},
        {UNKNOWN, UNKNOWN}
    },
    [STR]  = {
        {UNKNOWN, UNKNOWN},
        {UNKNOWN, UNKNOWN}
    }
};

// static inline bool dir_requires_operand(const Directive_Type dir) {
//     return dir_acc_tk[dir][0][0] != Token_Type::UNKNOWN;
// }
