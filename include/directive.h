#pragma once

#include "lexer.h"

typedef enum {
    ALIAS = 0,
    INCLUDE,
    STR,
    RES,
    COUNT // this is not a valid directive (used to known how many directives we have)
} Directive_Type;

constexpr static Token_Type dir_acc_tk[][2][2] = {
    [ALIAS]   = {
        {KEYWORD, UNKNOWN},
        {INTEGER, FP}
    },
    [INCLUDE] = {
        {STRING,  UNKNOWN},
        {UNKNOWN, UNKNOWN}
    },
    [STR]     = {
        {KEYWORD, UNKNOWN},
        {STRING, UNKNOWN}
    },
    [RES]     = {
        {KEYWORD, UNKNOWN},
        {INTEGER, UNKNOWN}
    }
};

static const char *dir_type_as_cstr(const Directive_Type& dir) USED_FUNCTION;
static const char *dir_type_as_cstr(const Directive_Type& dir) {
    switch (dir) {
    case Directive_Type::ALIAS:     return "alias";
    case Directive_Type::INCLUDE:   return "include";
    case Directive_Type::STR:       return "string";
    case Directive_Type::RES:       return "res";
    case Directive_Type::COUNT:
    default:
        std::cerr << "ERROR: Could not parse directive, unknown directive." << std::endl;
        exit(1);
    }
}

static Directive_Type str_as_dir(const std::string &str) USED_FUNCTION;
static Directive_Type str_as_dir(const std::string &str) {
    for (int dir_to_check = 0; dir_to_check < Directive_Type::COUNT; dir_to_check++) {
        // compare the strings
        if (!str.compare(dir_type_as_cstr((Directive_Type)dir_to_check))) {
            return (Directive_Type) dir_to_check;
        }
    }

    // should never happen
    return Directive_Type::COUNT;
}
