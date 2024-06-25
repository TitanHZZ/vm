#pragma once
#include <vector>
#include <unordered_set>

#include "lexer.h"
#include "program.h"

typedef struct {
    void *points_to;
    Token *token;
} Label;

typedef struct {
    size_t inst_idx;
    const Token &token;
} Unresolved_Label;

class Parser {
public:
    Parser(std::vector<Token> &tokens, Parser *parent = nullptr);
    void parse(Program *p = nullptr);

private:
    void parse_inst();
    void parse_label();
    void parse_directive();
    Nan_Box token_as_Nan_Box(const Token &token);
    const Token &next(const Token_Type acc_types[], size_t acc_types_size, bool ignore_keyword_type_check = false);

    size_t pos;
    std::vector<Token> &tokens;

    /*
     This data structures **need** to own the data as the Lexers and Parsers get destroyed in the recursive process
     of the 'include' directive. In the end, the first created Lexer and Parser, both need to have access to valid data.
     */
    std::vector<Inst> insts;
    std::vector<Nan_Box> memory;
    std::vector<Unresolved_Label> unresolved_labels;
    std::unordered_map<std::string, Token> alias;
    std::unordered_map<std::string, Label> labels;

    // used to stores the names for the included files to avoid circular file inclusion
    std::unordered_set<std::string> includes;

    Parser *parent;
};
